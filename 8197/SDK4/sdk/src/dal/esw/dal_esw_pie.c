/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008-2009
 * All rights reserved.
 *
 * $Revision: 30053 $
 * $Date: 2012-06-19 14:12:07 +0800 (Tue, 19 Jun 2012) $
 *
 * Purpose : Definition those public PIE APIs and its data type in the SDK .
 *
 * Feature : The file have include the following module and sub-modules
 *            1) Flow Classification
 *            2) Ingress ACL
 *            3) Egress ACL
 *            4) Egress VID Translation
 *            5) Range Check
 *            6) Field Selector
 *            7) Pattern Match
 *
 */
  
/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_pie.h>
#include <rtk/default.h>
#include <rtk/pie.h>

/*
 * Symbol Definition
 */
#define DAL_ESW_MAX_NUM_OF_PIE_PBLOCK            32
#define DAL_ESW_ENTRY_NUM_OF_PAYLOAD              2
#define DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD      5
#define DAL_ESW_PIE_COUNTER_BLOCK               128
#define DAL_ESW_BUFFER_UNIT_LENGTH_BITS          32    
#define DAL_ESW_DATA_BITS                         8
#define DAL_ESW_FILED_BITS                       16

#define DAL_ESW_ACTION_OUTER_VLAN_TAG_SIZE       18
#define DAL_ESW_ACTION_REDIRECT_SIZE             18
#define DAL_ESW_ACTION_INNER_VLAN_TAG_SIZE       17
#define DAL_ESW_ACTION_PRI_DP_SIZE               11
#define DAL_ESW_ACTION_SPID_SIZE                  9
#define DAL_ESW_ACTION_DSCP_REMARK_SIZE           9
#define DAL_ESW_ACTION_POLICER_SIZE               9
#define DAL_ESW_ACTION_OUTER_PRI_REMARK_SIZE   (8+1)  /* reserve one more bit to consist with policer */
#define DAL_ESW_ACTION_MIRROR_SIZE                3

#define DAL_ESW_ACTION_INFO_FIELD1_LENGTH        18
#define DAL_ESW_ACTION_INFO_FIELD2_LENGTH        18
#define DAL_ESW_ACTION_INFO_FIELD3_LENGTH        17
#define DAL_ESW_ACTION_INFO_FIELD4_LENGTH        11
#define DAL_ESW_ACTION_INFO_FIELD5_LENGTH         9

#define DAL_ESW_PIE_BIG_ENDIAN16(value)\
do {\
    value = (((value >> 8) & 0x00ff0000) | ((value << 8) & 0xff000000) | ((value >> 8) & 0x000000ff) | ((value << 8) & 0x0000ff00));\
} while (0)

#define DAL_ESW_PIE_BIG_ENDIAN32(value)\
do {\
    value = (((value >> 24) & 0x000000ff) | ((value >> 8) & 0x0000ff00) | ((value << 8) & 0x00ff0000) | ((value << 24) & 0xff000000));\
} while (0)

#define DAL_ESW_MAX_INFO_IDX                \
    (DAL_ESW_BUFFER_UNIT_LENGTH_BITS / DAL_ESW_DATA_BITS)
/* The macro is calaulate the '_data_len' bits need to use how many bytes */
#define DAL_ESW_DATA_WIDTH_GET(_data_len)                   \
    ((_data_len + DAL_ESW_DATA_BITS - 1) / DAL_ESW_DATA_BITS)
/* The macro is return the '_offset' location in which index of the uint8 array */
#define DAL_ESW_GET_INFO_IDX(_size, _offset)                \
    (DAL_ESW_DATA_WIDTH_GET(_size) - (_offset/DAL_ESW_DATA_BITS) - 1)

#define DAL_ESW_GET_INFO_OFFSET(_idx)                       \
    ((DAL_ESW_MAX_INFO_IDX - (_idx % DAL_ESW_MAX_INFO_IDX) - 1) * DAL_ESW_DATA_BITS)

#define SOFTWARE_CLR_OP

/*
 * Data Declaration
 */
static uint32               pie_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         pie_sem[RTK_MAX_NUM_OF_UNIT];

const static uint16 templatePayloadOffset_fieldIdx[DAL_ESW_MAX_NUM_OF_PIE_PBLOCK][DAL_ESW_ENTRY_NUM_OF_PAYLOAD] = {
                                                             {ESW_OFF0BLK0f, ESW_OFF1BLK0f}, {ESW_OFF0BLK1f, ESW_OFF1BLK1f}, 
                                                             {ESW_OFF0BLK2f, ESW_OFF1BLK2f}, {ESW_OFF0BLK3f, ESW_OFF1BLK3f}, 
                                                             {ESW_OFF0BLK4f, ESW_OFF1BLK4f}, {ESW_OFF0BLK5f, ESW_OFF1BLK5f}, 
                                                             {ESW_OFF0BLK6f, ESW_OFF1BLK6f}, {ESW_OFF0BLK7f, ESW_OFF1BLK7f}, 
                                                             {ESW_OFF0BLK8f, ESW_OFF1BLK8f}, {ESW_OFF0BLK9f, ESW_OFF1BLK9f}, 
                                                             {ESW_OFF0BLK10f, ESW_OFF1BLK10f}, {ESW_OFF0BLK11f, ESW_OFF1BLK11f}, 
                                                             {ESW_OFF0BLK12f, ESW_OFF1BLK12f}, {ESW_OFF0BLK13f, ESW_OFF1BLK13f}, 
                                                             {ESW_OFF0BLK14f, ESW_OFF1BLK14f}, {ESW_OFF0BLK15f, ESW_OFF1BLK15f}, 
                                                             {ESW_OFF0BLK16f, ESW_OFF1BLK16f}, {ESW_OFF0BLK17f, ESW_OFF1BLK17f}, 
                                                             {ESW_OFF0BLK18f, ESW_OFF1BLK18f}, {ESW_OFF0BLK19f, ESW_OFF1BLK19f}, 
                                                             {ESW_OFF0BLK20f, ESW_OFF1BLK20f}, {ESW_OFF0BLK21f, ESW_OFF1BLK21f}, 
                                                             {ESW_OFF0BLK22f, ESW_OFF1BLK22f}, {ESW_OFF0BLK23f, ESW_OFF1BLK23f}, 
                                                             {ESW_OFF0BLK24f, ESW_OFF1BLK24f}, {ESW_OFF0BLK25f, ESW_OFF1BLK25f}, 
                                                             {ESW_OFF0BLK26f, ESW_OFF1BLK26f}, {ESW_OFF0BLK27f, ESW_OFF1BLK27f}, 
                                                             {ESW_OFF0BLK28f, ESW_OFF1BLK28f}, {ESW_OFF0BLK29f, ESW_OFF1BLK29f},
                                                             {ESW_OFF0BLK30f, ESW_OFF1BLK30f}, {ESW_OFF0BLK31f, ESW_OFF1BLK31f}};                                            

const static uint16 lbolckGroupControl_fieldIdx[] = {ESW_PIEGCR0f, ESW_PIEGCR1f, ESW_PIEGCR2f, ESW_PIEGCR3f}; 

#if defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE)
typedef struct rtk_pie_entryTable_s
{ /* 320 bits */
    uint16   data_field0;    /*word0L*/
    uint16   data_field1;    /*word0H*/
    uint16   data_field2;    /*word1L*/
    uint16   data_field3;    /*word1H*/
    uint16   data_field4;    /*word2L*/
    uint16   data_field5;    /*word2H*/
    uint16   data_field6;    /*word3L*/
    uint16   data_field7;    /*word3H*/
    uint16   data_field8;    /*word4L*/
    uint16   valid;
    uint16   care_field0;
    uint16   care_field1;
    uint16   care_field2;
    uint16   care_field3;
    uint16   care_field4;
    uint16   care_field5;
    uint16   care_field6;
    uint16   care_field7;
    uint16   care_field8;
    uint16   care_reserved;    
} rtk_pie_entryTable_t;
#else
typedef struct rtk_pie_entryTable_s
{ /* 320 bits */
    uint16   data_field1;    /*word0H*/
    uint16   data_field0;    /*word0L*/
    uint16   data_field3;    /*word1H*/
    uint16   data_field2;    /*word1L*/
    uint16   data_field5;    /*word2H*/
    uint16   data_field4;    /*word2L*/
    uint16   data_field7;    /*word3H*/
    uint16   data_field6;    /*word3L*/
    uint16   valid;
    uint16   data_field8;    /*word4L*/
    uint16   care_field1;
    uint16   care_field0;
    uint16   care_field3;
    uint16   care_field2;
    uint16   care_field5;
    uint16   care_field4;
    uint16   care_field7;
    uint16   care_field6;
    uint16   care_reserved;   
    uint16   care_field8;
} rtk_pie_entryTable_t;
#endif

typedef enum rtk_pie_entryFieldType_e
{ 
    ENTRY_FIELD_TYPE0 = 0,
    ENTRY_FIELD_TYPE1,
    ENTRY_FIELD_TYPE2,
    ENTRY_FIELD_TYPE3,
    ENTRY_FIELD_TYPE4,
    ENTRY_FIELD_TYPE5,
    ENTRY_FIELD_TYPE6,
    ENTRY_FIELD_TYPE7,
    ENTRY_FIELD_TYPE8,
    ENTRY_FIELD_VALID,
    ENTRY_FIELD_END
} rtk_pie_entryFieldType_t;

typedef struct rtk_pie_entryFieldLocation_s
{
    uint32   template_field_type;
    uint32   field_offset;
    uint32   field_length;
    uint32   data_offset;
} rtk_pie_entryFieldlocation_t;

typedef struct rtk_pie_entryField_s
{
    uint32                          field_number;   /* locate in how many fields */
    rtk_pie_entryFieldlocation_t    *pField;
} rtk_pie_entryField_t;

rtk_pie_entryFieldlocation_t LOOKUP_PHASE_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0xe,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t SRC_PHY_PORT_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0x9,
        /* length */                 5,
        /* data offset */            0x0      
    },
};
rtk_pie_entryFieldlocation_t EXTRA_TAG_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0x6,
        /* length */                 1,
        /* data offset */            0x0     
    },
};
rtk_pie_entryFieldlocation_t TGL2_FORMAT_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0x7,
        /* length */                 2,
        /* data offset */            0x0      
    },
};
rtk_pie_entryFieldlocation_t TGL23_FORMAT_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0x4,
        /* length */                 2,
        /* data offset */            0x0       
    },
};
rtk_pie_entryFieldlocation_t TGL4_FORMAT_FIELDS[] = 
{
    {   
        /* template field type */    0,   /*FMT*/
        /* offset address */         0x1,
        /* length */                 3,
        /* data offset */            0x0       
    },
};
rtk_pie_entryFieldlocation_t SPM_FIELDS[] = 
{
    {   
        /* template field type */    37,   /*SPM (28-13)*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0xd, 
    },
    {   
        /* template field type */    38,   /*SPM (12-0)*/
        /* offset address */         0x3,
        /* length */                 13,
        /* data offset */            0x0       
    },    
};
rtk_pie_entryFieldlocation_t SPSM_FIELDS[] = 
{
    {   
        /* template field type */    39,   /*SPSM(15-0)*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0      
    },
};        
rtk_pie_entryFieldlocation_t DPM_FIELDS[] = 
{
    {   
        /* template field type */    40,   /*DPM (28-13)*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0xd,     
    },
    {   
        /* template field type */    41,   /*DPM (12-0)*/
        /* offset address */         0x3,
        /* length */                 13,  
        /* data offset */            0x0      
    },    
};
rtk_pie_entryFieldlocation_t DA_TYPE_FIELDS[] = 
{
    {   
        /* template field type */    38,   /*SPM (12-0)*/
        /* offset address */         0x1,
        /* length */                 2,
        /* data offset */            0x0
    },
};   
rtk_pie_entryFieldlocation_t CTLPKT_FIELDS[] = 
{
    {   
        /* template field type */    38,   /*SPM (12-0)*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t CPU_FIELDS[] = 
{
    {   
        /* template field type */    41,   /*DPM(12-0)*/
        /* offset address */         0x2,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t COPYTOCPU_FIELDS[] = 
{
    {   
        /* template field type */    41,   /*DPM(12-0)*/
        /* offset address */         0x1,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t DPN_FIELDS[] = 
{
    {   
        /* template field type */    59,   /*DPN*/
        /* offset address */         0xb,
        /* length */                 5,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t DMAC_FIELDS[] = 
{
    {   
        /* template field type */    3,   /*DMAC2[47:32]*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x20,      
    },
    {   
        /* template field type */    2,   /*DMAC1[31:16]*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,  
    },
    {   
        /* template field type */    1,   /*DMAC0[15:0]*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t SMAC_FIELDS[] = 
{
    {   
        /* template field type */    6,   /*SMAC2[47:32]*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x20,      
    },
    {   
        /* template field type */    5,   /*SMAC1[31:16]*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,        
    },
    {   
        /* template field type */    4,   /*SMAC0[15:0]*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t ETHERTYPE_FIELDS[] = 
{
    {   
        /* template field type */    7,   /*ETHERTYPE*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t OTAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    8,   /*OTAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};  
rtk_pie_entryFieldlocation_t DEI_EXIST_FIELDS[] = 
{
    {   
        /* template field type */    8,   /*OTAG*/
        /* offset address */         0xc,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t OTAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    8,   /*OTAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t ITAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    9,   /*ITAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t CFI_EXIST_FIELDS[] = 
{
    {   
        /* template field type */    9,   /*ITAG*/
        /* offset address */         0xc,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t ITAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    9,   /*ITAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_OTAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    10,   /*FWDOTAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_OTAG_CPU_TAG_FIELDS[] = 
{
    {   
        /* template field type */    10,   /*FWDOTAG*/
        /* offset address */         0xc,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_OTAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    10,   /*FWDOTAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_ITAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    11,   /*FWDITAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_ITAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    11,   /*FWDITAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_VID_PRI_FIELDS[] = 
{
    {   
        /* template field type */    12,   /*FWDVID*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FWD_VID_IVID_OR_OVID_FIELDS[] = 
{
    {   
        /* template field type */    12,   /*FWDVID*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t DP_FIELDS[] = 
{
    {   
        /* template field type */    42,   /*VIDRANG*/
        /* offset address */         0x2,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IPV4TOS_IPV6DS_FIELDS[] = 
{
    {   
        /* template field type */    13,   /*IPTOSPROTO*/
        /* offset address */         0x8,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IPV4PROTO_IPV6NH_FIELDS[] = 
{
    {   
        /* template field type */    13,   /*IPTOSPROTO*/
        /* offset address */         0x0,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t TTL_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x8,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t TTL_TYPE_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x6,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_DF_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x5,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_MF_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x4,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_NONZERO_OFFSET_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x3,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t REDIRECT_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x2,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_UCAST_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x1,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_MCAST_FIELDS[] = 
{
    {   
        /* template field type */    14,   /*IP4TTLFRAG*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t L4_SRC_PORT_FIELDS[] = 
{
    {   
        /* template field type */    15,   /*L4SPORT*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t L4_DST_PORT_FIELDS[] = 
{
    {   
        /* template field type */    16,   /*L4DPORT*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t TCP_FLAG_FIELDS[] = 
{
    {   
        /* template field type */    17,   /*TCPFLAG*/
        /* offset address */         0xa,
        /* length */                 6,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t ICMP_CODE_FIELDS[] = 
{
    {   
        /* template field type */    18,   /*ICMPCODETYPE*/
        /* offset address */         0x8,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t ICMP_TYPE_FIELDS[] = 
{
    {   
        /* template field type */    18,   /*ICMPCODETYPE*/
        /* offset address */         0x0,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IGMP_TYPE_FIELDS[] = 
{
    {   
        /* template field type */    19,   /*IGMPTYPE*/
        /* offset address */         0x8,
        /* length */                 8,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t SIP_FIELDS[] = 
{
    {   
        /* template field type */    21,   /*SIP1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,        
    },
    {   
        /* template field type */    20,   /*SIP0*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },    
};
rtk_pie_entryFieldlocation_t DIP_FIELDS[] = 
{
    {   
        /* template field type */    29,   /*DIP1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,     
    },
    {   
        /* template field type */    28,   /*DIP0*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x0
    },    
};
rtk_pie_entryFieldlocation_t IPV6_SIP_FIELDS[] = 
{
    {   
        /* template field type */    27,   /*SIP7*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x70,     
    },
    {   
        /* template field type */    26,   /*SIP6*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x60,   
    },
    {   
        /* template field type */    25,   /*SIP5*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x50,        
    },
    {   
        /* template field type */    24,   /*SIP4*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x40,         
    },    
    {   
        /* template field type */    23,   /*SIP3*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x30,     
    },
    {   
        /* template field type */    22,   /*SIP2*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x20,   
    },
    {   
        /* template field type */    21,   /*SIP1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,        
    },
    {   
        /* template field type */    20,   /*SIP0*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x0
    },    
};
rtk_pie_entryFieldlocation_t IPV6_DIP_FIELDS[] = 
{
    {   
        /* template field type */    35,   /*DIP7*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x70,    
    },
    {   
        /* template field type */    34,   /*DIP6*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x60,   
    },
    {   
        /* template field type */    33,   /*DIP5*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x50,      
    },
    {   
        /* template field type */    32,   /*DIP4*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x40,           
    },    
    {   
        /* template field type */    31,   /*DIP3*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x30,    
    },
    {   
        /* template field type */    30,   /*DIP2*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x20,           
    },
    {   
        /* template field type */    29,   /*DIP1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x10,  
    },
    {   
        /* template field type */    28,   /*DIP0*/
        /* offset address */         0x0,
        /* length */                 16, 
        /* data offset */            0x0
    },    
};
rtk_pie_entryFieldlocation_t FLOW_LABEL_FIELDS[] = 
{
    {   
        /* template field type */    36,   /*IPv6FLWH*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x4,      
    },
    {   
        /* template field type */    17,   /*TCPFLAG*/
        /* offset address */         0x6,
        /* length */                 4,  
        /* data offset */            0x0
    },   
};
rtk_pie_entryFieldlocation_t PPPOE_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t RTK_PROTO_FIELDS[] = 
{
    {   
        /* template field type */    42,    /*VIDRANG*/
        /* offset address */         0x4,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t TO_GUEST_VLAN_FIELDS[] = 
{
    {   
        /* template field type */    43,    /*GuestVLAN*/
        /* offset address */         0xf,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t KEEP_ORG_VID_FIELDS[] = 
{
    {   
        /* template field type */    43,    /*GuestVLAN*/
        /* offset address */         0x3,
        /* length */                 12,
        /* data offset */            0x0,      
    },
};
rtk_pie_entryFieldlocation_t VID_RANGE_HIT_FIELDS[] = 
{
    {   
        /* template field type */    42,    /*VIDRANG*/
        /* offset address */         0xf,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t VID_RANGE_FIELDS[] = 
{
    {   
        /* template field type */    42,    /*VIDRANG*/
        /* offset address */         0xa,
        /* length */                 5,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PORT_RANGE_FIELDS[] = 
{
    {   
        /* template field type */    44,    /*PORTRANG*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_RANGE0_FIELDS[] = 
{
    {   
        /* template field type */    45,    /*IPRANG*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0,        
    },
};
rtk_pie_entryFieldlocation_t IP_RANGE1_FIELDS[] = 
{
    {   
        /* template field type */    46,    /*IPRANG*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_RANGE2_FIELDS[] = 
{
    {   
        /* template field type */    47,    /*IPRANG*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t IP_RANGE3_FIELDS[] = 
{
    {   
        /* template field type */    48,    /*IPRANG*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PATTERN_MATCH0_FIELDS[] = 
{
    {   
        /* template field type */    49,    /*PatternMatch0*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PATTERN_MATCH1_FIELDS[] = 
{
    {   
        /* template field type */    50,    /*PatternMatch1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR1_FIELDS[] = 
{
    {   
        /* template field type */    42,    /*VIDRANG*/
        /* offset address */         0x9,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR2_FIELDS[] = 
{
    {   
        /* template field type */    42,    /*VIDRANG*/
        /* offset address */         0x8,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR1_0_FIELDS[] = 
{
    {   
        /* template field type */    51,    /*FieldDelector1_0*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR1_1_FIELDS[] = 
{
    {   
        /* template field type */    52,    /*FieldDelector1_1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR1_2_FIELDS[] = 
{
    {   
        /* template field type */    53,    /*FieldDelector1_2*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR1_3_FIELDS[] = 
{
    {   
        /* template field type */    54,    /*FieldDelector1_3*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR2_0_FIELDS[] = 
{
    {   
        /* template field type */    55,    /*FieldDelector2_0*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR2_1_FIELDS[] = 
{
    {   
        /* template field type */    56,    /*FieldDelector2_1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR2_2_FIELDS[] = 
{
    {   
        /* template field type */    57,    /*FieldDelector2_2*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t FIELD_SELECTOR2_3_FIELDS[] = 
{
    {   
        /* template field type */    58,    /*FieldDelector2_3*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PAYLOAD0_VALID_FIELDS[] = 
{
    {   
        /* template field type */    59,    /*DPN*/
        /* offset address */         0xa,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PAYLOAD1_VALID_FIELDS[] = 
{
    {   
        /* template field type */    59,    /*DPN*/
        /* offset address */         0x9,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PAYLOAD0_FIELDS[] = 
{
    {   
        /* template field type */    60,    /*Payload0*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t PAYLOAD1_FIELDS[] = 
{
    {   
        /* template field type */    61,    /*Payload1*/
        /* offset address */         0x0,
        /* length */                 16,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_LOOKUP_PHASE_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0xe,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_SRC_TRUNK_PHY_PORT_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0x9,
        /* length */                 5,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_L2_FORMAT_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0x5,
        /* length */                 4,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_L34_FORMAT_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0x1,
        /* length */                 4,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_EXTRA_TAG_FIELDS[] = 
{
    {   
        /* template field type */    0,    /*FMT*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DMAC_INDEX_FIELDS[] = 
{
    {   
        /* template field type */    1,    /*DMAC0*/
        /* offset address */         0x2,
        /* length */                 14,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_IP_UCAST_FIELDS[] = 
{
    {   
        /* template field type */    1,    /*DMAC0*/
        /* offset address */         0x1,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_IP_MCAST_FIELDS[] = 
{
    {   
        /* template field type */    1,    /*DMAC0*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_RRCP_FIELDS[] = 
{
    {   
        /* template field type */    2,    /*RRCPINFO*/
        /* offset address */         0xf,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_RRCP_TYPE_FIELDS[] = 
{
    {   
        /* template field type */    2,    /*RRCPINFO*/
        /* offset address */         0xd,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DOTAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    8,    /*OTAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_OTAG_EXIST_FIELDS[] = 
{
    {   
        /* template field type */    8,    /*OTAG*/
        /* offset address */         0xc,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DOTAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    8,    /*OTAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DITAG_PRI_FIELDS[] = 
{
    {   
        /* template field type */    9,    /*INTAG*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_ITAG_EXIST_FIELDS[] = 
{
    {   
        /* template field type */    9,    /*INTAG*/
        /* offset address */         0xc,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DITAG_VID_FIELDS[] = 
{
    {   
        /* template field type */    9,    /*INTAG*/
        /* offset address */         0x0,
        /* length */                 12,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_PRI_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0xd,
        /* length */                 3,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DP_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0xb,
        /* length */                 2,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DSCP_RMK_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0xa,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DSCP_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0x4,
        /* length */                 6,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_RX_CPU_TAG_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0x3,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_RX_RSPAN_FIELDS[] = 
{
    {   
        /* template field type */    10,    /*DPPRI*/
        /* offset address */         0x2,
        /* length */                 1,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t EVT_DPN_FIELDS[] = 
{
    {   
        /* template field type */    59,    /*DPN(28-13)*/
        /* offset address */         0xb,
        /* length */                 5,
        /* data offset */            0x0
    },
};
rtk_pie_entryFieldlocation_t VAILD_FIELDS[] = 
{
    {   
        /* template field type */    100,    /*100 for VALID bit*/
        /* offset address */         0x0,
        /* length */                 1,
        /* data offset */            0x0
    },
};

rtk_pie_entryField_t rtk_pie_entry_field_list[] =
{
    {   /* field name                FIELD_LOOKUP_PHASE */
        /* field number */           1,
        /* field pointer */          LOOKUP_PHASE_FIELDS
    },
    {   /* field name                FIELD_SRC_PHY_PORT */
        /* field number */           1,   
        /* field pointer */          SRC_PHY_PORT_FIELDS
    },
    {   /* field name                FIELD_EXTRA_TAG */
        /* field number */           1,
        /* field pointer */          EXTRA_TAG_FIELDS
    },
    {   /* field name                FIELD_TGL2_FORMAT */
        /* field number */           1,
        /* field pointer */          TGL2_FORMAT_FIELDS
    },
    {   /* field name                FIELD_TGL23_FORMAT */
        /* field number */           1,
        /* field pointer */          TGL23_FORMAT_FIELDS
    },
    {   /* field name                FIELD_TGL4_FORMAT */
        /* field number */           1,
        /* field pointer */          TGL4_FORMAT_FIELDS
    },
    {   /* field name                FIELD_SPM */
        /* field number */           2,
        /* field pointer */          SPM_FIELDS
    },
    {   /* field name                FIELD_SPSM */
        /* field number */           1,
        /* field pointer */          SPSM_FIELDS
    },
    {   /* field name                FIELD_DPM */
        /* field number */           2,
        /* field pointer */          DPM_FIELDS
    },
    {   /* field name                FIELD_DA_TYPE */
        /* field number */           1,
        /* field pointer */          DA_TYPE_FIELDS
    },
    {   /* field name                FIELD_CTLPKT */
        /* field number */           1,
        /* field pointer */          CTLPKT_FIELDS
    },   
    {   /* field name                FIELD_CPU */
        /* field number */           1,
        /* field pointer */          CPU_FIELDS
    }, 
    {   /* field name                FIELD_COPYTOCPU */
        /* field number */           1,
        /* field pointer */          COPYTOCPU_FIELDS
    },
    {   /* field name                FIELD_DPN */
        /* field number */           1,
        /* field pointer */          DPN_FIELDS
    },
    {   /* field name                FIELD_DMAC */
        /* field number */           3,
        /* field pointer */          DMAC_FIELDS
    },
    {   /* field name                FIELD_SMAC */
        /* field number */           3,
        /* field pointer */          SMAC_FIELDS
    },
    {   /* field name                FIELD_ETHERTYPE */
        /* field number */           1,
        /* field pointer */          ETHERTYPE_FIELDS
    },
    {   /* field name                FIELD_OTAG_PRI */
        /* field number */           1,
        /* field pointer */          OTAG_PRI_FIELDS
    },
    {   /* field name                FIELD_DEI_EXIST */
        /* field number */           1,
        /* field pointer */          DEI_EXIST_FIELDS
    },
    {   /* field name                FIELD_OTAG_VID */
        /* field number */           1,
        /* field pointer */          OTAG_VID_FIELDS
    },
    {   /* field name                FIELD_ITAG_PRI */
        /* field number */           1,
        /* field pointer */          ITAG_PRI_FIELDS
    },
    {   /* field name                FIELD_CFI_EXIST */
        /* field number */           1,
        /* field pointer */          CFI_EXIST_FIELDS
    },
    {   /* field name                FIELD_ITAG_VID */
        /* field number */           1,
        /* field pointer */          ITAG_VID_FIELDS
    },
    {   /* field name                FIELD_FWD_OTAG_PRI */
        /* field number */           1,
        /* field pointer */          FWD_OTAG_PRI_FIELDS
    },
    {   /* field name                FIELD_FWD_CPU_TAG */
        /* field number */           1,
        /* field pointer */          FWD_OTAG_CPU_TAG_FIELDS
    },
    {   /* field name                FIELD_FWD_OTAG_VID */
        /* field number */           1,
        /* field pointer */          FWD_OTAG_VID_FIELDS
    },
    {   /* field name                FIELD_FWD_ITAG_PRI */
        /* field number */           1,
        /* field pointer */          FWD_ITAG_PRI_FIELDS
    },
    {   /* field name                FIELD_FWD_ITAG_VID */
        /* field number */           1,
        /* field pointer */          FWD_ITAG_VID_FIELDS
    },
    {   /* field name                FIELD_FWD_VID_PRI */
        /* field number */           1,
        /* field pointer */          FWD_VID_PRI_FIELDS
    },
    {   /* field name                FIELD_FWD_VID_IVID_OR_OVID */
        /* field number */           1,
        /* field pointer */          FWD_VID_IVID_OR_OVID_FIELDS
    },
    {   /* field name                FIELD_DP */
        /* field number */           1,
        /* field pointer */          DP_FIELDS
    },
    {   /* field name                FIELD_IPV4TOS_IPV6DS */
        /* field number */           1,
        /* field pointer */          IPV4TOS_IPV6DS_FIELDS
    },
    {   /* field name                FIELD_IPV4PROTO_IPV6NH */
        /* field number */           1,
        /* field pointer */          IPV4PROTO_IPV6NH_FIELDS
    },
    {   /* field name                FIELD_TTL */
        /* field number */           1,
        /* field pointer */          TTL_FIELDS
    },
    {   /* field name                FIELD_TTL_TYPE */
        /* field number */           1,
        /* field pointer */          TTL_TYPE_FIELDS
    },
    {   /* field name                FIELD_IP_DF */
        /* field number */           1,
        /* field pointer */          IP_DF_FIELDS
    },
    {   /* field name                FIELD_IP_MF */
        /* field number */           1,
        /* field pointer */          IP_MF_FIELDS
    },
    {   /* field name                FIELD_IP_NONZERO_OFFSET */
        /* field number */           1,
        /* field pointer */          IP_NONZERO_OFFSET_FIELDS
    },
    {   /* field name                FIELD_REDIRECT */
        /* field number */           1,
        /* field pointer */          REDIRECT_FIELDS
    },
    {   /* field name                FIELD_IP_UCAST */
        /* field number */           1,
        /* field pointer */          IP_UCAST_FIELDS
    },
    {   /* field name                FIELD_IP_MCAST */
        /* field number */           1,
        /* field pointer */          IP_MCAST_FIELDS
    },
    {   /* field name                FIELD_L4_SRC_PORT */
        /* field number */           1,
        /* field pointer */          L4_SRC_PORT_FIELDS
    },
    {   /* field name                FIELD_L4_DST_PORT */
        /* field number */           1,
        /* field pointer */          L4_DST_PORT_FIELDS
    },
    {   /* field name                FIELD_TCP_FLAG */
        /* field number */           1,
        /* field pointer */          TCP_FLAG_FIELDS
    },
    {   /* field name                FIELD_ICMP_CODE */
        /* field number */           1,
        /* field pointer */          ICMP_CODE_FIELDS
    },
    {   /* field name                FIELD_ICMP_TYPE */
        /* field number */           1,
        /* field pointer */          ICMP_TYPE_FIELDS
    },
    {   /* field name                FIELD_IGMP_TYPE */
        /* field number */           1,
        /* field pointer */          IGMP_TYPE_FIELDS
    },
    {   /* field name                FIELD_SIP */
        /* field number */           2,
        /* field pointer */          SIP_FIELDS
    },
    {   /* field name                FIELD_DIP */
        /* field number */           2,
        /* field pointer */          DIP_FIELDS
    },
    {   /* field name                FIELD_IPV6_SIP */
        /* field number */           8,
        /* field pointer */          IPV6_SIP_FIELDS
    },
    {   /* field name                FIELD_IPV6_DIP */
        /* field number */           8,
        /* field pointer */          IPV6_DIP_FIELDS
    },
    {   /* field name                FIELD_FLOW_LABEL */
        /* field number */           2,
        /* field pointer */          FLOW_LABEL_FIELDS
    },
    {   /* field name                FIELD_PPPOE */
        /* field number */           1,
        /* field pointer */          PPPOE_FIELDS
    },
    {   /* field name                FIELD_RTK_PROTO */
        /* field number */           1,
        /* field pointer */          RTK_PROTO_FIELDS
    },
    {   /* field name                FIELD_TO_GUEST_VLAN */
        /* field number */           1,
        /* field pointer */          TO_GUEST_VLAN_FIELDS
    },
    {   /* field name                FIELD_KEEP_ORG_VID */
        /* field number */           1,
        /* field pointer */          KEEP_ORG_VID_FIELDS
    },
    {   /* field name                FIELD_VID_RANGE_HIT */
        /* field number */           1,
        /* field pointer */          VID_RANGE_HIT_FIELDS
    },
    {   /* field name                FIELD_VID_RANGE */
        /* field number */           1,
        /* field pointer */          VID_RANGE_FIELDS
    },
    {   /* field name                FIELD_PORT_RANGE */
        /* field number */           1,
        /* field pointer */          PORT_RANGE_FIELDS
    },
    {   /* field name                FIELD_IP_RANGE0 */
        /* field number */           1,
        /* field pointer */          IP_RANGE0_FIELDS
    },
    {   /* field name                FIELD_IP_RANGE1 */
        /* field number */           1,
        /* field pointer */          IP_RANGE1_FIELDS
    },
    {   /* field name                FIELD_IP_RANGE2 */
        /* field number */           1,
        /* field pointer */          IP_RANGE2_FIELDS
    },
    {   /* field name                FIELD_IP_RANGE3 */
        /* field number */           1,
        /* field pointer */          IP_RANGE3_FIELDS
    },
    {   /* field name                FIELD_PATTERN_MATCH0 */
        /* field number */           1,
        /* field pointer */          PATTERN_MATCH0_FIELDS
    },
    {   /* field name                FIELD_PATTERN_MATCH1 */
        /* field number */           1,
        /* field pointer */          PATTERN_MATCH1_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR1 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR1_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR2 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR2_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR1_0 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR1_0_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR1_1 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR1_1_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR1_2 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR1_2_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR1_3 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR1_3_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR2_0 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR2_0_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR2_1 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR2_1_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR2_2 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR2_2_FIELDS
    },
    {   /* field name                FIELD_FIELD_SELECTOR2_3 */
        /* field number */           1,
        /* field pointer */          FIELD_SELECTOR2_3_FIELDS
    },
    {   /* field name                FIELD_PAYLOAD0_VALID */
        /* field number */           1,
        /* field pointer */          PAYLOAD0_VALID_FIELDS
    },
    {   /* field name                FIELD_PAYLOAD1_VALID */
        /* field number */           1,
        /* field pointer */          PAYLOAD1_VALID_FIELDS
    },
    {   /* field name                FIELD_PAYLOAD0 */
        /* field number */           1,
        /* field pointer */          PAYLOAD0_FIELDS
    },
    {   /* field name                FIELD_PAYLOAD1 */
        /* field number */           1,
        /* field pointer */          PAYLOAD1_FIELDS
    },
    {   /* field name                FIELD_EVT_LOOKUP_PHASE */
        /* field number */           1,
        /* field pointer */          EVT_LOOKUP_PHASE_FIELDS
    },
    {   /* field name                FIELD_EVT_SRC_TRUNK_PHY_PORT */
        /* field number */           1,
        /* field pointer */          EVT_SRC_TRUNK_PHY_PORT_FIELDS
    },
    {   /* field name                FIELD_EVT_L2_FORMAT */
        /* field number */           1,
        /* field pointer */          EVT_L2_FORMAT_FIELDS
    },
    {   /* field name                FIELD_EVT_L34_FORMAT */
        /* field number */           1,
        /* field pointer */          EVT_L34_FORMAT_FIELDS
    },    
    {   /* field name                FIELD_EVT_EXTRA_TAG */
        /* field number */           1,
        /* field pointer */          EVT_EXTRA_TAG_FIELDS
    },
    {   /* field name                FIELD_EVT_DMAC_INDEX */
        /* field number */           1,
        /* field pointer */          EVT_DMAC_INDEX_FIELDS
    },
    {   /* field name                FIELD_EVT_IP_UCAST */
        /* field number */           1,
        /* field pointer */          EVT_IP_UCAST_FIELDS
    },
    {   /* field name                FIELD_EVT_IP_MCAST */
        /* field number */           1,
        /* field pointer */          EVT_IP_MCAST_FIELDS
    },
    {   /* field name                FIELD_EVT_RRCP */
        /* field number */           1,
        /* field pointer */          EVT_RRCP_FIELDS
    },
    {   /* field name                FIELD_EVT_RRCP_TYPE */
        /* field number */           1,
        /* field pointer */          EVT_RRCP_TYPE_FIELDS
    },
    {   /* field name                FIELD_EVT_DOTAG_PRI */
        /* field number */           1,
        /* field pointer */          EVT_DOTAG_PRI_FIELDS
    },
    {   /* field name                FIELD_EVT_OTAG_EXIST */
        /* field number */           1,
        /* field pointer */          EVT_OTAG_EXIST_FIELDS
    },
    {   /* field name                FIELD_EVT_DOTAG_VID */
        /* field number */           1,
        /* field pointer */          EVT_DOTAG_VID_FIELDS
    },
    {   /* field name                FIELD_EVT_DITAG_PRI */
        /* field number */           1,
        /* field pointer */          EVT_DITAG_PRI_FIELDS
    },
    {   /* field name                FIELD_EVT_ITAG_EXIST */
        /* field number */           1,
        /* field pointer */          EVT_ITAG_EXIST_FIELDS
    },
    {   /* field name                FIELD_EVT_DITAG_VID */
        /* field number */           1,
        /* field pointer */          EVT_DITAG_VID_FIELDS
    },
    {   /* field name                FIELD_EVT_PRI */
        /* field number */           1,
        /* field pointer */          EVT_PRI_FIELDS
    },
    {   /* field name                FIELD_EVT_DP */
        /* field number */           1,
        /* field pointer */          EVT_DP_FIELDS
    },
    {   /* field name                FIELD_EVT_DSCP_RMK */
        /* field number */           1,
        /* field pointer */          EVT_DSCP_RMK_FIELDS
    },
    {   /* field name                FIELD_EVT_DSCP */
        /* field number */           1,
        /* field pointer */          EVT_DSCP_FIELDS
    },
    {   /* field name                FIELD_EVT_RX_CPU_TAG */
        /* field number */           1,
        /* field pointer */          EVT_RX_CPU_TAG_FIELDS
    },
    {   /* field name                FIELD_EVT_RX_RSPAN */
        /* field number */           1,
        /* field pointer */          EVT_RX_RSPAN_FIELDS
    },
    {   /* field name                FIELD_EVT_DPN */
        /* field number */           1,
        /* field pointer */          EVT_DPN_FIELDS
    },
    {   /* field name                FIELD_VAILD */
        /* field number */           1,
        /* field pointer */          VAILD_FIELDS
    },
};

/*
 * Macro Declaration
 */

/* semaphore handling */
#define PIE_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(pie_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_PIE), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)

#define PIE_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(pie_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_PIE), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

#define DAL_ESW_PIE_BIT_VALUE_MAX(bits)   ((1 << bits) - 1)

/*
 * Function Declaration
 */
static int32 dal_esw_pie_pieActionInfoField_get(uint32 unit,pie_act_entry_t *pPie_action, uint32 action_info_field, uint32 *value);
static int32 dal_esw_pie_pieActionCheckMixedCounterType(uint32  unit, rtk_pie_id_t  action_idx, rtk_pie_statisticType_t type);
static int32 dal_esw_pie_pieActionInfoField_set(uint32 unit, pie_act_entry_t *pPie_action, uint32 action_info_field, uint32 value);
static int32 dal_esw_pie_pieCheckPhaseFieldType(uint32 unit, rtk_pie_phase_t phase, rtk_pie_id_t template_idx);
static int32 dal_esw_pie_pieCheckFieldLocation(rtk_pie_id_t field_idx, rtk_pie_templateFiledType_t field_type);

/* Function Name:
 *      dal_esw_pie_init
 * Description:
 *      Initialize PIE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize PIE module before calling any PIE APIs.
 */
int32
dal_esw_pie_init(uint32 unit)
{
    int32   ret = RT_ERR_FAILED;
    rtk_pie_clearBlockContent_t content;

    pie_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    pie_sem[unit] = osal_sem_mutex_create();
    if (0 == pie_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    pie_init[unit] = INIT_COMPLETED;

    /* Clear PIE entry, action and counter */
    content.start_idx = 0;
    content.end_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit) - 1;
    if ((ret = dal_esw_pie_pieRuleEntry_del(unit, &content)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "dal_esw_pie_pieRuleEntry_del all failed");
        return RT_ERR_FAILED;
    }

    if ((ret = dal_esw_pie_pieRuleAction_del(unit, &content)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "dal_esw_pie_pieRuleAction_del all failed");
        return RT_ERR_FAILED;
    }

    if ((ret = dal_esw_pie_pieStat_clearAll(unit)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "dal_esw_pie_pieStat_clearAll failed");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_esw_pie_init */        

/* Function Name:
 *      dal_esw_pie_pieRuleEntryFieldSize_get
 * Description:
 *      Get the field size of PIE entry.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_size - field size of PIE entry.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type 
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      The unit of size is bit.
 */
int32
dal_esw_pie_pieRuleEntryFieldSize_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_size)
{
    uint32  index;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d type=%d", unit, type);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pField_size), RT_ERR_NULL_POINTER);
    
    *pField_size = 0;
    /* get field size by type from database */
    for (index = 0; index < rtk_pie_entry_field_list[type].field_number; index++)
    {
        *pField_size += rtk_pie_entry_field_list[type].pField[index].field_length;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pField_size=%d", *pField_size);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntryFieldSize_get */

/* Function Name:
 *      dal_esw_pie_pieRuleFieldId_get
 * Description:
 *      Get the field ID of specified field type.
 * Input:
 *      unit        - unit id
 *      type        - type of entry field
 * Output:
 *      pField_id - field ID of the specified field type.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type 
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Get the field Id.
 */
int32
dal_esw_pie_pieRuleFieldId_get(uint32 unit, rtk_pie_fieldType_t type, uint32 *pField_id)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d type=%d", unit, type);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pField_id), RT_ERR_NULL_POINTER);
    
    /* get field id by type from database */
    *pField_id = rtk_pie_entry_field_list[type].pField[0].template_field_type;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pField_id=%d", *pField_id);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleFieldId_get */

/* Function Name:
 *      dal_esw_pie_pieRuleEntrySize_get
 * Description:
 *      Get the rule entry size of PIE.
 * Input:
 *      unit        - unit id
 * Output:
 *      pEntry_size - rule entry size of PIE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The unit of size is bit.
 */
int32
dal_esw_pie_pieRuleEntrySize_get(uint32 unit, uint32 *pEntry_size)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d pEntry_size=%x", unit, pEntry_size);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pEntry_size), RT_ERR_NULL_POINTER);
    
    *pEntry_size = sizeof(rtk_pie_entryTable_t) * 8;
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntrySize_get */

/* Function Name:
 *      dal_esw_pie_pieRuleEntryField_get
 * Description:
 *      Get the field data from specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 * Output:
 *      pData         - field data
 *      pMask         - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_PHASE        - invalid PIE phase
 *      RT_ERR_ENTRY_INDEX      - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type  
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieRuleEntryField_get(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    uint32  pblock_idx;
    uint32  field_idx;
    uint32  field_number;
    uint32  template_idx;
    uint32  total_length = 0;
    rtk_pie_template_t template;
    uint32  field_match = FALSE;
    uint16  field_data;
    uint16  field_mask = 0;
    rtk_pie_entryTable_t *pEntry_table = NULL;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, phase=%d, entry_idx=%d, type=%d", unit, phase, entry_idx, type);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    pEntry_table = (rtk_pie_entryTable_t *) pEntry_buffer;    
    /*caculate entry in which pblock */
    pblock_idx = entry_idx/HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    /* find out pblock binding template index*/
    dal_esw_pie_pieTemplateSelector_get(unit, pblock_idx, phase, &template_idx);
    /* get field types configured in template*/
    osal_memset(&template, 0, sizeof(rtk_pie_template_t));
    dal_esw_pie_pieUserTemplate_get(unit, template_idx, &template);

    if (FIELD_VAILD == type)
    {
        *pData = pEntry_table->valid;
        *pMask = 1;
        return RT_ERR_OK;
    }

    /* search template to check all field types */
    field_match = FALSE;
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++)
    {   
        for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++)
        {   /* check if the input type defined in template */
            if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])
            {
                field_match = TRUE;
                /* if find the match field type, then get field data*/
                switch (field_idx)
                { 
                    case ENTRY_FIELD_TYPE0:
                        field_data = pEntry_table->data_field0;
                        field_mask = pEntry_table->care_field0;
                        break;
                    case ENTRY_FIELD_TYPE1:
                        field_data = pEntry_table->data_field1;
                        field_mask = pEntry_table->care_field1;
                        break;
                    case ENTRY_FIELD_TYPE2:
                        field_data = pEntry_table->data_field2;
                        field_mask = pEntry_table->care_field2;
                        break;            
                    case ENTRY_FIELD_TYPE3:
                        field_data = pEntry_table->data_field3;
                        field_mask = pEntry_table->care_field3;
                        break;
                    case ENTRY_FIELD_TYPE4:
                        field_data = pEntry_table->data_field4;
                        field_mask = pEntry_table->care_field4;
                        break;
                    case ENTRY_FIELD_TYPE5:
                        field_data = pEntry_table->data_field5;
                        field_mask = pEntry_table->care_field5;
                        break;
                    case ENTRY_FIELD_TYPE6:
                        field_data = pEntry_table->data_field6;
                        field_mask = pEntry_table->care_field6;
                        break;
                    case ENTRY_FIELD_TYPE7:
                        field_data = pEntry_table->data_field7;
                        field_mask = pEntry_table->care_field7;
                        break;
                    case ENTRY_FIELD_TYPE8:
                        field_data = pEntry_table->data_field8;
                        field_mask = pEntry_table->care_field8;
                        break;
                    case ENTRY_FIELD_VALID:
                        field_data = pEntry_table->valid;
                        break;            
                    default:
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field type error");
                        return RT_ERR_FAILED;
                } /* end of switch (field_idx) */               

                /* find out related data field from field data */
                if (FIELD_VAILD == type)
                {
                    *pData = field_data;
                    *pMask = 1;
                }
                else
                {
                    #if defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE)
                    {
                     uint32  *pTmp_data;
                     uint32  *pTmp_mask;       
                     uint32  field_offset, data_offset, field_length, first_data_len;          

                     field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                     data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                     field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                     total_length += field_length; 
                     pTmp_data = (uint32 *)pData + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                     pTmp_mask = (uint32 *)pMask + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                     
                     if (((data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS) + field_length) > DAL_ESW_BUFFER_UNIT_LENGTH_BITS)
                     {/* process fragment data, ex:flow label */
                          /* get the front of data(the length is from data offet to field end) from buffer field to put into the front of data. */
                          first_data_len = DAL_ESW_BUFFER_UNIT_LENGTH_BITS - (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                          *pTmp_data |= (((field_data >> field_offset) & ((1 << first_data_len) - 1)) << data_offset);
                          /* get the rear of data(the length is field length - first_data_len) from buffer field to put into the rear of data. */
                          pTmp_data++;
                          *pTmp_data |= (((field_data >> field_offset) >> first_data_len) & ((1 << (field_length-first_data_len)) - 1));

                          *pTmp_mask |= (((field_mask >> field_offset) & ((1 << first_data_len) - 1)) << data_offset);
                          pTmp_mask++;
                          *pTmp_mask |= (((field_mask >> field_offset) >> first_data_len) & ((1 << (field_length-first_data_len)) - 1));
                     }
                     else
                     {
                        *pTmp_data |= (((field_data >> field_offset) & ((1 << field_length) - 1)) << data_offset);
                        *pTmp_mask |= (((field_mask >> field_offset) & ((1 << field_length) - 1)) << data_offset);
                     }
                    }
                    #else /* defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) */
                    /* find out related data field from field data */
                    {
                        uint32  *pTmp_data;
                        uint32  *pTmp_mask;       
                        uint32  field_offset, data_offset, field_length;          
                        uint32  i, data_field = 0;
                        uint32  tmp_offset;
                        uint32  info_offset, info_idx, info_len, mask_len;
                        uint32  info_field_offset;
                         
                        field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                        data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                        field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                        //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "field_offset=%d, data_offset=%d, data_offset=%d", field_offset, data_offset, field_length);
    
                        dal_esw_pie_pieRuleEntryFieldSize_get(unit, type, &total_length);
                        //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "type=%d, total_length=%d", type, total_length);
                        
                        info_idx = DAL_ESW_GET_INFO_IDX(total_length, data_offset);
                        info_len = field_length;
                        //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "info_idx=%d, info_len=%d", info_idx, info_len);
                        for (i = 0; i < DAL_ESW_DATA_WIDTH_GET(field_length); ++i)
                        {
                            tmp_offset = ((info_idx - i) / DAL_ESW_MAX_INFO_IDX);
                            pTmp_data = (uint32 *)pData + tmp_offset;
                            pTmp_mask = (uint32 *)pMask + tmp_offset;
        
                            info_offset = DAL_ESW_GET_INFO_OFFSET((info_idx - i))+(data_offset%DAL_ESW_DATA_BITS);
        
                            if (info_len > DAL_ESW_DATA_BITS)
                            {
                                mask_len = DAL_ESW_DATA_BITS;
                                info_len -= DAL_ESW_DATA_BITS;
                            }
                            else
                            {
                                mask_len = info_len;
                            }
        
                            info_field_offset = field_offset + (i * DAL_ESW_DATA_BITS);
        
                            data_field = ((1 << mask_len) - 1) << info_offset;
                            *pTmp_data &= ~data_field;
                            *pTmp_mask &= ~data_field;
        
                            *pTmp_data |= ((field_data >> info_field_offset) & ((1 << mask_len) - 1)) << info_offset;
                            *pTmp_mask |= ((field_mask >> info_field_offset) & ((1 << mask_len) - 1)) << info_offset;
                        }
                    }
                    #endif

                } /* end of if (FIELD_VAILD == type) */
                 
            }/* end of if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])*/
 
        }/* for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++) */

    } /* for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++) */
    
    /* can't find then return */
    if ( field_match != TRUE)
    {
        return RT_ERR_PIE_FIELD_TYPE;
    }

    #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    {
        uint32  index;
        uint8   temp=0;
        uint8   *pData_temp=NULL, *pMask_temp=NULL;
    
        for (index = 0; index <= (total_length / DAL_ESW_BUFFER_UNIT_LENGTH_BITS); index++)
        {
            pData_temp = pData + (index * (DAL_ESW_BUFFER_UNIT_LENGTH_BITS / 8));
            temp = *pData_temp;
            *pData_temp = *(pData_temp + 3);
            *(pData_temp + 3) = temp;
            temp = *(pData_temp+1);
            *(pData_temp + 1) = *(pData_temp + 2);
            *(pData_temp + 2) = temp;
    
            pMask_temp = pMask + (index * (DAL_ESW_BUFFER_UNIT_LENGTH_BITS / 8));
            temp = *pMask_temp;
            *pMask_temp = *(pMask_temp + 3);
            *(pMask_temp + 3) = temp;
            temp = *(pMask_temp + 1);
            *(pMask_temp + 1) = *(pMask_temp + 2);
            *(pMask_temp + 2) = temp;
        }
    }
    #endif


    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x, pMask=%x", *pData, *pMask);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntryField_get */    

/* Function Name:
 *      dal_esw_pie_pieRuleEntryField_set
 * Description:
 *      Set the field data to specified PIE entry buffer.
 * Input:
 *      unit          - unit id
 *      phase         - PIE lookup phase
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 *      type          - field type
 *      pData         - field data
 *      pMask         - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_PHASE        - invalid PIE phase 
 *      RT_ERR_ENTRY_INDEX      - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type  
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieRuleEntryField_set(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    uint32  pblock_idx;
    uint32  field_idx;
    uint32  field_number;
    uint32  template_idx;
    rtk_pie_template_t template;
    uint32  field_match = FALSE;
    rtk_pie_entryTable_t *pEntry_table = NULL;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, phase=%d, entry_idx=%d, type=%d, pData=%x, pMask=%x", unit, phase, entry_idx, type, pData, pMask);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    pEntry_table = (rtk_pie_entryTable_t *) pEntry_buffer;    
    /*caculate entry in which pblock */
    pblock_idx = entry_idx/HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    /* find out pblock binding template index*/
    dal_esw_pie_pieTemplateSelector_get(unit, pblock_idx, phase, &template_idx);
    /* get field types configured in template*/
    osal_memset(&template, 0, sizeof(rtk_pie_template_t));
    dal_esw_pie_pieUserTemplate_get(unit, template_idx, &template);

    if (FIELD_VAILD == type)
    {
        pEntry_table->valid = *pData;
        return RT_ERR_OK;
    }

    /* search template to check all field types */
    field_match = FALSE;
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++)
    {   
        for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++)
        {   /* check if the input type defined in template */
            if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])
            {
                uint32  field_offset, data_offset, field_length;
                uint16  field_data = 0;
                uint16  field_mask = 0;
             #if defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE)
             {
                field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                 
                field_match = TRUE;

                /* find out related data field from field data */
                {
                     uint32  *pTmp_data;
                     uint32  *pTmp_mask;       
                     #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
                     uint32 ednData=0;
                     uint32 ednMask=0;
                     #endif

                     pTmp_data = (uint32 *)pData + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                     pTmp_mask = (uint32 *)pMask + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);

                     #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
                     ednData = *pTmp_data;
                     ednData = (((ednData & 0xff) << 24) | ((ednData & 0xff00) << 8) | ((ednData & 0xff0000) >> 8) | ((ednData & 0xff000000) >> 24));
                     pTmp_data = &ednData;
                     ednMask = *pTmp_mask;
                     ednMask = (((ednMask & 0xff) << 24) | ((ednMask & 0xff00) << 8) | ((ednMask & 0xff0000) >> 8) | ((ednMask & 0xff000000) >> 24));
                     pTmp_mask = &ednMask;
                     #endif

                     field_data = ((*pTmp_data >> (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS)) & ((1 << field_length)-1)) << field_offset;
                     field_mask = ((*pTmp_mask >> (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS)) & ((1 << field_length)-1)) << field_offset;
                } /* end of if (FIELD_VAILD == type) */
             }
             #else /* defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) */
             {
                uint32  *pTmp_data;
                uint32  *pTmp_mask;
                uint32  field_size, tmp_offset;
                uint32  info_offset, info_idx, info_len, mask_len, i;
                uint32  info_field_offset;
                //uint32  temp_data = 0;
                //uint32  temp_mask = 0;
                #if defined(CONFIG_SDK_ENDIAN_LITTLE)
                uint32 ednData=0;
                uint32 ednMask=0;
                #endif
                 
                field_match = TRUE;
                field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;

                dal_esw_pie_pieRuleEntryFieldSize_get(unit, type, &field_size);
                //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "type=%d, field_size=%d", type, field_size);

                /* form the field data in chip view from user input */
                info_idx = DAL_ESW_GET_INFO_IDX(field_size, data_offset);
                info_len = field_length;
                //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "info_idx=%d, info_len=%d", info_idx, info_len);
                field_data = field_mask = 0;

                /* find out related data field from field data */
                for (i = 0; i < DAL_ESW_DATA_WIDTH_GET(field_length); ++i)
                {
                    tmp_offset = ((info_idx - i) / DAL_ESW_MAX_INFO_IDX);
                    pTmp_data = (uint32 *)pData + tmp_offset;
                    pTmp_mask = (uint32 *)pMask + tmp_offset;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "*pTmp_data=0x%x, *pTmp_mask=0x%x", *pTmp_data, *pTmp_mask);
                    #if defined(CONFIG_SDK_ENDIAN_LITTLE)
                    ednData = *pTmp_data;
                    ednData = (((ednData & 0xff) << 24) | ((ednData & 0xff00) << 8) | ((ednData & 0xff0000) >> 8) | ((ednData & 0xff000000) >> 24));
                    pTmp_data = &ednData;
                    ednMask = *pTmp_mask;
                    ednMask = (((ednMask & 0xff) << 24) | ((ednMask & 0xff00) << 8) | ((ednMask & 0xff0000) >> 8) | ((ednMask & 0xff000000) >> 24));
                    pTmp_mask = &ednMask;
                    #endif

                    info_offset = DAL_ESW_GET_INFO_OFFSET((info_idx - i))+(data_offset%DAL_ESW_DATA_BITS);

                    if (info_len > DAL_ESW_DATA_BITS)
                    {
                        mask_len = DAL_ESW_DATA_BITS;
                        info_len -= DAL_ESW_DATA_BITS;
                    }
                    else
                    {
                        mask_len = info_len;
                    }

                    info_field_offset = field_offset + (i * DAL_ESW_DATA_BITS);

                    field_data |= ((*pTmp_data >> info_offset) & ((1 << mask_len)-1)) << info_field_offset;
                    field_mask |= ((*pTmp_mask >> info_offset) & ((1 << mask_len)-1)) << info_field_offset;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "field_data[%d]=0x%x, field_mask[%d]=0x%x", i, field_data, i, field_mask);
                }
             }
             #endif

                /* if find the match field type, then get field data*/
                switch (field_idx)
                { 
                    case ENTRY_FIELD_TYPE0:
                        pEntry_table->data_field0 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field0 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field0 |= field_data;
                        pEntry_table->care_field0 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE1:
                        pEntry_table->data_field1 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field1 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field1 |= field_data;
                        pEntry_table->care_field1 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE2:
                        pEntry_table->data_field2 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field2 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field2 |= field_data;
                        pEntry_table->care_field2 |= field_mask;

                        break;            
                    case ENTRY_FIELD_TYPE3:
                        pEntry_table->data_field3 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field3 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field3 |= field_data;
                        pEntry_table->care_field3 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE4:
                        pEntry_table->data_field4 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field4 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field4 |= field_data;
                        pEntry_table->care_field4 |= field_mask;
                       break;
                    case ENTRY_FIELD_TYPE5:
                        pEntry_table->data_field5 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field5 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field5 |= field_data;
                        pEntry_table->care_field5 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE6:
                        pEntry_table->data_field6 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field6 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field6 |= field_data;
                        pEntry_table->care_field6 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE7:
                        pEntry_table->data_field7 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field7 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field7 |= field_data;
                        pEntry_table->care_field7 |= field_mask;
                        break;
                    case ENTRY_FIELD_TYPE8:
                        pEntry_table->data_field8 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->care_field8 &= ~(((1 << field_length)-1) << field_offset);
                        pEntry_table->data_field8 |= field_data;
                        pEntry_table->care_field8 |= field_mask;
                        break;
                    default:
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field type error");
                        return RT_ERR_FAILED;
                } /* end of switch (field_idx) */               
                 
            }/* end of if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])*/
 
        }/* for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++) */

    } /* for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++) */

    /* can't find then return */
    if (field_match != TRUE)
    {
        return RT_ERR_PIE_FIELD_TYPE;
    }

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntryField_set */    

/* Function Name:
 *      dal_esw_pie_pieRuleEntryField_read
 * Description:
 *      Read the field data from specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 * Output:
 *      pData     - field data
 *      pMask     - field mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_PHASE        - invalid PIE phase 
 *      RT_ERR_ENTRY_INDEX      - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type  
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieRuleEntryField_read(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    int32   ret;
    uint32  pblock_idx;
    uint32  field_idx;
    uint32  field_number;
    uint32  template_idx;
    uint32  table_index;
    uint32  value;
    uint32 total_length = 0;
    rtk_pie_template_t template;
    uint32  field_match = FALSE;
    uint16  field_data;
    uint16  field_mask = 0;
    pie_rule_entry_t    pie_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, phase=%d, entry_idx=%d, type=%d", unit, phase, entry_idx, type);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    /*caculate entry in which pblock */
    pblock_idx = entry_idx/HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    /* find out pblock binding template index*/
    dal_esw_pie_pieTemplateSelector_get(unit, pblock_idx, phase, &template_idx);
    /* get field types configured in template*/
    osal_memset(&template, 0, sizeof(rtk_pie_template_t));
    dal_esw_pie_pieUserTemplate_get(unit, template_idx, &template);

    /*translate entry index to table index*/
    table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));

    if (FIELD_VAILD == type)
    {
        PIE_SEM_LOCK(unit);
        
        if ((ret = table_read(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
                       
        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_VALIDf, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }

        PIE_SEM_UNLOCK(unit);

        *pData = value;
        *pMask = 1;
        return RT_ERR_OK;
    }

    /* search template to check all field types */
    field_match = FALSE;
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++)
    {   
        for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++)
        {   /* check if the input type defined in template */
            if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])
            {
                field_match = TRUE;
                /* if find the match field type, then get field data*/
                /* get filed data from chip */
                PIE_SEM_LOCK(unit);
                
                if ((ret = table_read(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
                {
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                    return ret;
                }
                
                switch (field_idx)
                { 
                    case ENTRY_FIELD_TYPE0:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = (value & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = (value & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE1:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = ((value >> 16) & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = ((value >> 16) & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE2:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = (value & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = (value & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE3:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = ((value >> 16) & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = ((value >> 16) & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE4:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = (value & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = (value & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE5:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = ((value >> 16) & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = ((value >> 16) & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE6:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = (value & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = (value & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE7:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = ((value >> 16) & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = ((value >> 16) & 0xffff);
                        break;
                        
                    case ENTRY_FIELD_TYPE8:
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_data = (value & 0xffff);
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        field_mask = (value & 0xffff);
                        break;
                    default:
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field type error");
                        return RT_ERR_FAILED;
                } /* end of switch (field_idx) */

                PIE_SEM_UNLOCK(unit);
                
                #if defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE)
                /* find out related data field from field data */
                {
                     uint32  *pTmp_data;
                     uint32  *pTmp_mask;       
                     uint32  field_offset, data_offset, field_length, first_data_len;          
                     
                     field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                     data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                     field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                     total_length += field_length;                     
                     pTmp_data = (uint32 *)pData + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                     pTmp_mask = (uint32 *)pMask + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                     
                     if (((data_offset%DAL_ESW_BUFFER_UNIT_LENGTH_BITS) + field_length) > DAL_ESW_BUFFER_UNIT_LENGTH_BITS)
                     {/* process fragment data, ex:flow label */
                          /* get the front of data(the length is from data offet to field end) from buffer field to put into the front of data. */
                          first_data_len = DAL_ESW_BUFFER_UNIT_LENGTH_BITS - (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                          *pTmp_data |= (((field_data >> field_offset) & ((1 << first_data_len) - 1)) << data_offset);
                          /* get the rear of data(the length is field length - first_data_len) from buffer field to put into the rear of data. */
                          pTmp_data++;
                          *pTmp_data |= (((field_data >> field_offset) >> first_data_len) & ((1 << (field_length-first_data_len)) - 1));

                          *pTmp_mask |= (((field_mask >> field_offset) & ((1 << first_data_len) - 1)) << data_offset);
                          pTmp_mask++;
                          *pTmp_mask |= (((field_mask >> field_offset) >> first_data_len) & ((1 << (field_length-first_data_len)) - 1));
                     }
                     else
                     {
                        *pTmp_data |= (((field_data >> field_offset) & ((1 << field_length) - 1)) << data_offset);
                        *pTmp_mask |= (((field_mask >> field_offset) & ((1 << field_length) - 1)) << data_offset);
                     }
                }
                #else /* defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) */
                /* find out related data field from field data */
                {
                    uint32  *pTmp_data;
                    uint32  *pTmp_mask;       
                    uint32  field_offset, data_offset, field_length;          
                    uint32  i, data_field = 0;
                    uint32  tmp_offset;
                    uint32  info_offset, info_idx, info_len, mask_len;
                    uint32  info_field_offset;
                     
                    field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                    data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                    field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "field_offset=%d, data_offset=%d, data_offset=%d", field_offset, data_offset, field_length);

                    dal_esw_pie_pieRuleEntryFieldSize_get(unit, type, &total_length);
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "type=%d, total_length=%d", type, total_length);
                    
                    info_idx = DAL_ESW_GET_INFO_IDX(total_length, data_offset);
                    info_len = field_length;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "info_idx=%d, info_len=%d", info_idx, info_len);
                    for (i = 0; i < DAL_ESW_DATA_WIDTH_GET(field_length); ++i)
                    {
                        tmp_offset = ((info_idx - i) / DAL_ESW_MAX_INFO_IDX);
                        pTmp_data = (uint32 *)pData + tmp_offset;
                        pTmp_mask = (uint32 *)pMask + tmp_offset;
    
                        info_offset = DAL_ESW_GET_INFO_OFFSET((info_idx - i))+(data_offset%DAL_ESW_DATA_BITS);
    
                        if (info_len > DAL_ESW_DATA_BITS)
                        {
                            mask_len = DAL_ESW_DATA_BITS;
                            info_len -= DAL_ESW_DATA_BITS;
                        }
                        else
                        {
                            mask_len = info_len;
                        }
    
                        info_field_offset = field_offset + (i * DAL_ESW_DATA_BITS);
    
                        data_field = ((1 << mask_len) - 1) << info_offset;
                        *pTmp_data &= ~data_field;
                        *pTmp_mask &= ~data_field;
    
                        *pTmp_data |= ((field_data >> info_field_offset) & ((1 << mask_len) - 1)) << info_offset;
                        *pTmp_mask |= ((field_mask >> info_field_offset) & ((1 << mask_len) - 1)) << info_offset;
                    }
                }
                #endif
                 
            }/* end of if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])*/
 
        }/* for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++) */

    } /* for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++) */
    
    /* can't find then return */
    if ( field_match != TRUE)
    {
        return RT_ERR_PIE_FIELD_TYPE;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x, pMask=%x", *pData, *pMask);

    #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    {
        uint32  index;
        uint8   temp=0;
        uint8   *pData_temp=NULL, *pMask_temp=NULL;
    
        for (index = 0; index <= (total_length / DAL_ESW_BUFFER_UNIT_LENGTH_BITS); index++)
        {
            pData_temp = pData + (index * (DAL_ESW_BUFFER_UNIT_LENGTH_BITS / 8));
            temp = *pData_temp;
            *pData_temp = *(pData_temp + 3);
            *(pData_temp + 3) = temp;
            temp = *(pData_temp+1);
            *(pData_temp + 1) = *(pData_temp + 2);
            *(pData_temp + 2) = temp;
    
            pMask_temp = pMask + (index * (DAL_ESW_BUFFER_UNIT_LENGTH_BITS / 8));
            temp = *pMask_temp;
            *pMask_temp = *(pMask_temp + 3);
            *(pMask_temp + 3) = temp;
            temp = *(pMask_temp + 1);
            *(pMask_temp + 1) = *(pMask_temp + 2);
            *(pMask_temp + 2) = temp;
        }
    }
    #endif
    
    return RT_ERR_OK;
}  /* end of dal_esw_pie_pieRuleEntryField_read */

/* Function Name:
 *      dal_esw_pie_pieRuleEntryField_write
 * Description:
 *      Write the field data to specified PIE entry.
 * Input:
 *      unit      - unit id
 *      phase     - PIE lookup phase
 *      entry_idx - PIE entry index
 *      type      - field type
 *      pData     - field data
 *      pMask     - field mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PIE_PHASE        - invalid PIE phase 
 *      RT_ERR_ENTRY_INDEX      - invalid entry index
 *      RT_ERR_PIE_FIELD_TYPE   - invalid entry field type  
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      1) The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieRuleEntryField_write(
    uint32              unit,
    rtk_pie_phase_t     phase,
    rtk_pie_id_t        entry_idx,
    rtk_pie_fieldType_t type,
    uint8               *pData,
    uint8               *pMask)
{
    int32   ret;
    uint32  pblock_idx;
    uint32  field_idx;
    uint32  field_number;
    uint32  template_idx;
    uint32  table_index;
    uint32  value;
    rtk_pie_template_t template;
    uint32  field_match = FALSE;
    pie_rule_entry_t    pie_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, phase=%d, entry_idx=%d, type=%d, pData=%x, pMask=%x", unit, phase, entry_idx, type, pData, pMask);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((type >= PIE_FIELD_END), RT_ERR_PIE_FIELD_TYPE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pMask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    /*caculate entry in which pblock */
    pblock_idx = entry_idx/HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    /* find out pblock binding template index*/
    dal_esw_pie_pieTemplateSelector_get(unit, pblock_idx, phase, &template_idx);
    /* get field types configured in template*/
    osal_memset(&template, 0, sizeof(rtk_pie_template_t));
    dal_esw_pie_pieUserTemplate_get(unit, template_idx, &template);

    /*translate entry index to table index*/
    table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));

    if (FIELD_VAILD == type)
    {
        PIE_SEM_LOCK(unit);
        
        if ((ret = table_read(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        value = *pData;
        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_VALIDf, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }                        
    
        /* set entry to chip */
        if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        PIE_SEM_UNLOCK(unit);
        return RT_ERR_OK;
    }
    
    /* search template to check all field types */
    field_match = FALSE;
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++)
    {   
        for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++)
        {   /* check if the input type defined in template */
            if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])
            {
            #if defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE)
                uint32  field_offset, data_offset, field_length;
                uint32  temp_data = 0;
                uint32  temp_mask = 0;
                uint16  field_data = 0;
                uint16  field_mask = 0;
                 
                field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;
                 
                field_match = TRUE;

                /* find out related data field from field data */
                {
                    uint32  *pTmp_data;
                    uint32  *pTmp_mask;       
                    #if defined(CONFIG_SDK_ENDIAN_BIG)
                    uint32 ednData=0;
                    uint32 ednMask=0;
                    #endif
                    
                    pTmp_data = (uint32 *)pData + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                    pTmp_mask = (uint32 *)pMask + (data_offset / DAL_ESW_BUFFER_UNIT_LENGTH_BITS);
                    
                    #if defined(CONFIG_SDK_ENDIAN_BIG)
                    ednData = *pTmp_data;
                    ednData = (((ednData & 0xff) << 24) | ((ednData & 0xff00) << 8) | ((ednData & 0xff0000) >> 8) | ((ednData & 0xff000000) >> 24));
                    pTmp_data = &ednData;
                    ednMask = *pTmp_mask;
                    ednMask = (((ednMask & 0xff) << 24) | ((ednMask & 0xff00) << 8) | ((ednMask & 0xff0000) >> 8) | ((ednMask & 0xff000000) >> 24));
                    pTmp_mask = &ednMask;
                    #endif
                    
                    field_data = ((*pTmp_data >> (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS)) & ((1 << field_length)-1)) << field_offset;
                    field_mask = ((*pTmp_mask >> (data_offset % DAL_ESW_BUFFER_UNIT_LENGTH_BITS)) & ((1 << field_length)-1)) << field_offset;

                }
            #else /* defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) */
                uint32  field_offset, data_offset, field_length;
                uint32  *pTmp_data;
                uint32  *pTmp_mask;
                uint32  field_size, tmp_offset;
                uint32  info_offset, info_idx, info_len, mask_len, i;
                uint32  info_field_offset;
                uint32  temp_data = 0;
                uint32  temp_mask = 0;
                uint16  field_data;
                uint16  field_mask;
                #if defined(CONFIG_SDK_ENDIAN_LITTLE)
                uint32 ednData=0;
                uint32 ednMask=0;
                #endif
                 
                field_match = TRUE;
                field_offset = rtk_pie_entry_field_list[type].pField[field_number].field_offset;
                data_offset = rtk_pie_entry_field_list[type].pField[field_number].data_offset;
                field_length = rtk_pie_entry_field_list[type].pField[field_number].field_length;

                dal_esw_pie_pieRuleEntryFieldSize_get(unit, type, &field_size);
                //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "type=%d, field_size=%d", type, field_size);

                /* form the field data in chip view from user input */
                info_idx = DAL_ESW_GET_INFO_IDX(field_size, data_offset);
                info_len = field_length;
                //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "info_idx=%d, info_len=%d", info_idx, info_len);
                field_data = field_mask = 0;

                /* find out related data field from field data */
                for (i = 0; i < DAL_ESW_DATA_WIDTH_GET(field_length); ++i)
                {
                    tmp_offset = ((info_idx - i) / DAL_ESW_MAX_INFO_IDX);
                    pTmp_data = (uint32 *)pData + tmp_offset;
                    pTmp_mask = (uint32 *)pMask + tmp_offset;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "*pTmp_data=0x%x, *pTmp_mask=0x%x", *pTmp_data, *pTmp_mask);
                    #if defined(CONFIG_SDK_ENDIAN_LITTLE)
                    ednData = *pTmp_data;
                    ednData = (((ednData & 0xff) << 24) | ((ednData & 0xff00) << 8) | ((ednData & 0xff0000) >> 8) | ((ednData & 0xff000000) >> 24));
                    pTmp_data = &ednData;
                    ednMask = *pTmp_mask;
                    ednMask = (((ednMask & 0xff) << 24) | ((ednMask & 0xff00) << 8) | ((ednMask & 0xff0000) >> 8) | ((ednMask & 0xff000000) >> 24));
                    pTmp_mask = &ednMask;
                    #endif

                    info_offset = DAL_ESW_GET_INFO_OFFSET((info_idx - i))+(data_offset%DAL_ESW_DATA_BITS);

                    if (info_len > DAL_ESW_DATA_BITS)
                    {
                        mask_len = DAL_ESW_DATA_BITS;
                        info_len -= DAL_ESW_DATA_BITS;
                    }
                    else
                    {
                        mask_len = info_len;
                    }

                    info_field_offset = field_offset + (i * DAL_ESW_DATA_BITS);

                    field_data |= ((*pTmp_data >> info_offset) & ((1 << mask_len)-1)) << info_field_offset;
                    field_mask |= ((*pTmp_mask >> info_offset) & ((1 << mask_len)-1)) << info_field_offset;
                    //RT_DBG(LOG_WARNING, (MOD_DAL|MOD_PIE), "field_data[%d]=0x%x, field_mask[%d]=0x%x", i, field_data, i, field_mask);
                }
            #endif

                /* if find the match field type, then get field data*/
                /* get filed data from chip */
                PIE_SEM_LOCK(unit);
                
                if ((ret = table_read(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
                {
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                    return ret;
                }
                switch (field_idx)
                { 
                    case ENTRY_FIELD_TYPE0:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = (value & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0xffff0000;
                        value |= temp_data;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                     
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = (value & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0xffff0000;
                        value |= temp_mask;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE1:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = ((value >> 16) & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0x0000ffff;
                        value |= (temp_data << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = ((value >> 16) & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0x0000ffff;
                        value |= (temp_mask << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_31_0f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE2:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = (value & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0xffff0000;
                        value |= temp_data;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = (value & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0xffff0000;
                        value |= temp_mask;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                              
                    case ENTRY_FIELD_TYPE3:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = ((value >> 16) & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0x0000ffff;
                        value |= (temp_data << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = ((value >> 16) & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0x0000ffff;
                        value |= (temp_mask << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_63_32f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE4:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = (value & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0xffff0000;
                        value |= temp_data;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = (value & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0xffff0000;
                        value |= temp_mask;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                       
                    case ENTRY_FIELD_TYPE5:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = ((value >> 16) & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0x0000ffff;
                        value |= (temp_data << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = ((value >> 16) & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0x0000ffff;
                        value |= (temp_mask << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_95_64f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE6:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = (value & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0xffff0000;
                        value |= temp_data;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = (value & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0xffff0000;
                        value |= temp_mask;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE7:
                       /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = ((value >> 16) & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0x0000ffff;
                        value |= (temp_data << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = ((value >> 16) & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0x0000ffff;
                        value |= (temp_mask << 16);
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_127_96f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                        
                    case ENTRY_FIELD_TYPE8:
                        /* data */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_DATA_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_data = (value & 0xffff);
                        temp_data &= ~(((1 << field_length)-1) << field_offset);
                        temp_data |= field_data;
                        value &= 0xffff0000;
                        value |= temp_data;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_DATA_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        
                        /* mask */
                        if ((ret = table_field_get(unit, ESW_PIEt, ESW_PIE_CAREBIT_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        temp_mask = (value & 0xffff);
                        temp_mask &= ~(((1 << field_length)-1) << field_offset);
                        temp_mask |= field_mask;
                        value &= 0xffff0000;
                        value |= temp_mask;
                        if ((ret = table_field_set(unit, ESW_PIEt, ESW_PIE_CAREBIT_143_128f, &value, (uint32 *) &pie_entry)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                        break;
                    default:
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field type error");
                        return RT_ERR_FAILED;
                } /* end of switch (field_idx) */               

                /* set entry to chip */
                if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
                {
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                    return ret;
                }

                PIE_SEM_UNLOCK(unit);
            }/* end of if (rtk_pie_entry_field_list[type].pField[field_number].template_field_type == template.field[field_idx])*/
 
        }/* for (field_number = 0; field_number < rtk_pie_entry_field_list[type].field_number; field_number++) */

    } /* for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx ++) */
    
    /* can't find then return */
    if (field_match != TRUE)
    {
        return RT_ERR_PIE_FIELD_TYPE;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntryField_write */

/* Function Name:
 *      dal_esw_pie_piePreDefinedRuleEntry_get
 * Description:
 *      Get the pre-defined entry data from buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry 
 * Output:
 *      pPredefined_entry - pre-defined entry data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_piePreDefinedRuleEntry_get(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    uint32                  value;
    rtk_pie_entryTable_t    *pEntry_table = NULL;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pEntry_buffer=%x, pPredefined_entry=%x", 
                                          unit, pEntry_buffer, pPredefined_entry);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pPredefined_entry), RT_ERR_NULL_POINTER);
    
    pEntry_table = (rtk_pie_entryTable_t *) pEntry_buffer; 

    #if 0 //(defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    /* for big endian, fix me */
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field0);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field0);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field1);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field1);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field2);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field2);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field3);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field3);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field4);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field4);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field5);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field5);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field6);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field6);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field7);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field7);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field8);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field8);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->valid);
    #endif

    pPredefined_entry->valid = pEntry_table->valid & 0x1;
    pPredefined_entry->lookup_phase = (pEntry_table->data_field8 >> 14) & 0x3;
    pPredefined_entry->lookup_phase_care = (pEntry_table->care_field8 >> 14) & 0x3;
    
    pPredefined_entry->src_phy_port = (pEntry_table->data_field8 >> 9) & 0x1f;
    pPredefined_entry->src_phy_port_care = (pEntry_table->care_field8 >> 9) & 0x1f;

    value = (pEntry_table->data_field8 >> 7) & 0x3;
    switch (value)
    {
        case 0:
            pPredefined_entry->tgl2fmt = L2FMT_NO_TAG;
            break;
            
        case 1:
            pPredefined_entry->tgl2fmt = L2FMT_INNER_TAG;
            break;
            
        case 2:
            pPredefined_entry->tgl2fmt = L2FMT_OUTER_TAG;
            break;
            
        case 3:
            pPredefined_entry->tgl2fmt = L2FMT_BOTH_INNER_OUTER_TAG;
            break;
            
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TGL2FMT value error");
            return RT_ERR_FAILED;
    }
    pPredefined_entry->tgl2fmt_care = (pEntry_table->care_field8 >> 7) & 0x3;

    pPredefined_entry->extra_tag = (pEntry_table->data_field8 >> 6) & 0x1;
    pPredefined_entry->extra_tag_care = (pEntry_table->care_field8 >> 6) & 0x1;

    value = (pEntry_table->data_field8 >> 4) & 0x3;
    switch (value)
    {
        case 0:
            pPredefined_entry->tgl23fmt = L23FMT_ARP;
            break;
            
        case 1:
            pPredefined_entry->tgl23fmt = L23FMT_L2;
            break;
            
        case 2:
            pPredefined_entry->tgl23fmt = L23FMT_IPV4;
            break;
            
        case 3:
            pPredefined_entry->tgl23fmt = L23FMT_IPV6;
            break;
            
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L23FMT value error");
            return RT_ERR_FAILED;
    }
    pPredefined_entry->tgl23fmt_care = (pEntry_table->care_field8 >> 4) & 0x3;

    value = (pEntry_table->data_field8 >> 1) & 0x7;
    switch (value)
    {
        case 0:
            pPredefined_entry->tgl4fmt = L4FMT_UNKNOWN;
            break;
            
        case 1:
            pPredefined_entry->tgl4fmt = L4FMT_TCP;
            break;
            
        case 2:
            pPredefined_entry->tgl4fmt = L4FMT_UDP;
            break;
            
        case 3:
            pPredefined_entry->tgl4fmt = L4FMT_ICMP;
            break;

        case 4:
            pPredefined_entry->tgl4fmt = L4FMT_IGMP;
            break;

        case 5:
            pPredefined_entry->tgl4fmt = L4FMT_ICMPV6;
            break;
                                    
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L4FMT value error");
            return RT_ERR_FAILED;
    }
    pPredefined_entry->tgl4fmt_care = (pEntry_table->care_field8 >> 1) & 0x7;

    pPredefined_entry->pppoe = pEntry_table->data_field8 & 0x1;
    pPredefined_entry->pppoe_care = pEntry_table->care_field8 & 0x1;

    pPredefined_entry->ttl_type = (pEntry_table->data_field6 >> 6) & 0x3;
    pPredefined_entry->ttl_type_care = (pEntry_table->care_field6 >> 6) & 0x3;

    pPredefined_entry->dst_vid = (pEntry_table->data_field6 >> 8) & 0xff;
    pPredefined_entry->dst_vid |= ((pEntry_table->data_field7 & 0xf) << 8);
    pPredefined_entry->dst_vid_care = (pEntry_table->care_field6 >> 8) & 0xff;
    pPredefined_entry->dst_vid_care |= ((pEntry_table->care_field7 & 0xf) << 8);

    pPredefined_entry->src_vid = (pEntry_table->data_field7 >> 4) & 0xfff;
    pPredefined_entry->src_vid_care = (pEntry_table->care_field7 >> 4) & 0xfff;

    switch (pPredefined_entry->tgl23fmt)
    {
        case L23FMT_ARP:
            if (L4FMT_UNKNOWN == pPredefined_entry->tgl4fmt)
            {/* ARP */
                pPredefined_entry->un.arp.sip = pEntry_table->data_field0 & 0xffff;
                pPredefined_entry->un.arp.sip |= ((uint32) pEntry_table->data_field1 << 16);
                pPredefined_entry->un.arp.sip_care = pEntry_table->care_field0 & 0xffff;
                pPredefined_entry->un.arp.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                pPredefined_entry->un.arp.dip = pEntry_table->data_field2 & 0xffff;
                pPredefined_entry->un.arp.dip |= ((uint32) pEntry_table->data_field3 << 16);
                pPredefined_entry->un.arp.dip_care = pEntry_table->care_field2 & 0xffff;
                pPredefined_entry->un.arp.dip_care |= ((uint32) pEntry_table->care_field3 << 16);
            }
            break;
            
        case L23FMT_IPV4:
            
            switch (pPredefined_entry->tgl4fmt)
            {
                case L4FMT_UNKNOWN:
                    /* IPv4 */
                    pPredefined_entry->un.ipv4.sip = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv4.sip |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv4.sip_care = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv4.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                    pPredefined_entry->un.ipv4.dip = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv4.dip |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv4.dip_care = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv4.dip_care |= ((uint32) pEntry_table->care_field3 << 16);

                    pPredefined_entry->un.ipv4.protocol = (pEntry_table->data_field5 >> 8) & 0xff;
                    pPredefined_entry->un.ipv4.protocol_care = (pEntry_table->care_field5 >> 8) & 0xff;

                    pPredefined_entry->un.ipv4.tos = pEntry_table->data_field5 & 0xff;
                    pPredefined_entry->un.ipv4.tos_care = pEntry_table->care_field5 & 0xff;

                    pPredefined_entry->un.ipv4.ttl = (pEntry_table->data_field4 >> 8) & 0xff;
                    pPredefined_entry->un.ipv4.ttl_care = (pEntry_table->care_field4 >> 8) & 0xff;
                    break;
            
                case L4FMT_TCP:
                    /* IPv4 TCP */
                    pPredefined_entry->un.ipv4_tcp.sip = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.sip |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv4_tcp.sip_care = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                    pPredefined_entry->un.ipv4_tcp.dip = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.dip |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv4_tcp.dip_care = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.dip_care |= ((uint32) pEntry_table->care_field3 << 16);

                    pPredefined_entry->un.ipv4_tcp.src_port = pEntry_table->data_field5 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.src_port_care = pEntry_table->care_field5 & 0xffff;

                    pPredefined_entry->un.ipv4_tcp.dst_port = pEntry_table->data_field4 & 0xffff;
                    pPredefined_entry->un.ipv4_tcp.dst_port_care = pEntry_table->care_field4 & 0xffff;

                    pPredefined_entry->un.ipv4_tcp.tcp_flag = pEntry_table->data_field6 & 0x3f;
                    pPredefined_entry->un.ipv4_tcp.tcp_flag_care = pEntry_table->care_field6 & 0x3f;
                    break;
                    
                case L4FMT_UDP:
                    /* IPv4 UDP */
                    pPredefined_entry->un.ipv4_udp.sip = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.sip |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv4_udp.sip_care = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                    pPredefined_entry->un.ipv4_udp.dip = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.dip |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv4_udp.dip_care = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.dip_care |= ((uint32) pEntry_table->care_field3 << 16);

                    pPredefined_entry->un.ipv4_udp.src_port = pEntry_table->data_field5 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.src_port_care = pEntry_table->care_field5 & 0xffff;

                    pPredefined_entry->un.ipv4_udp.dst_port = pEntry_table->data_field4 & 0xffff;
                    pPredefined_entry->un.ipv4_udp.dst_port_care = pEntry_table->care_field4 & 0xffff;
                    break;

                case L4FMT_ICMP:
                    /* IPv4 ICMP */
                    pPredefined_entry->un.ipv4_icmp.sip = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_icmp.sip |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv4_icmp.sip_care = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_icmp.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                    pPredefined_entry->un.ipv4_icmp.dip = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_icmp.dip |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv4_icmp.dip_care = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_icmp.dip_care |= ((uint32) pEntry_table->care_field3 << 16);

                    pPredefined_entry->un.ipv4_icmp.icmp_type = (pEntry_table->data_field5 >> 8) & 0xff;
                    pPredefined_entry->un.ipv4_icmp.icmp_type_care = (pEntry_table->care_field5 >> 8) & 0xff;

                    pPredefined_entry->un.ipv4_icmp.icmp_code = pEntry_table->data_field5 & 0xff;
                    pPredefined_entry->un.ipv4_icmp.icmp_code_care = pEntry_table->care_field5 & 0xff;
                    break;

                case L4FMT_IGMP:
                    /* IPv4 IGMP */
                    pPredefined_entry->un.ipv4_igmp.sip = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_igmp.sip |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv4_igmp.sip_care = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv4_igmp.sip_care |= ((uint32) pEntry_table->care_field1 << 16);

                    pPredefined_entry->un.ipv4_igmp.dip = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_igmp.dip |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv4_igmp.dip_care = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv4_igmp.dip_care |= ((uint32) pEntry_table->care_field3 << 16);

                    pPredefined_entry->un.ipv4_igmp.igmp_type = pEntry_table->data_field4 & 0xff;
                    pPredefined_entry->un.ipv4_igmp.igmp_type_care = pEntry_table->care_field4 & 0xff;

                    pPredefined_entry->un.ipv4_igmp.group_ip = (pEntry_table->data_field4 >> 8) & 0xff;
                    pPredefined_entry->un.ipv4_igmp.group_ip |= ((uint32) pEntry_table->data_field5 << 8);
                    pPredefined_entry->un.ipv4_igmp.group_ip |= (((uint32) pEntry_table->data_field6 & 0xf) << 24);
                    pPredefined_entry->un.ipv4_igmp.group_ip_care = (pEntry_table->care_field4 >> 8) & 0xff;
                    pPredefined_entry->un.ipv4_igmp.group_ip_care |= ((uint32) pEntry_table->care_field5 << 8);
                    pPredefined_entry->un.ipv4_igmp.group_ip_care |= (((uint32) pEntry_table->care_field6 & 0xf) << 24);
                    break;

                default:
                    break;
            }            
            break;

        case L23FMT_IPV6:

            switch (pPredefined_entry->tgl4fmt)
            {
                case L4FMT_UNKNOWN:
                    /* IPv6 */
                    pPredefined_entry->un.ipv6.ipv6_dip[3] = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv6.ipv6_dip[3] |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv6.ipv6_dip[2] = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv6.ipv6_dip[2] |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv6.ipv6_dip[1] = 0;
                    pPredefined_entry->un.ipv6.ipv6_dip[0] = 0;
                    pPredefined_entry->un.ipv6.ipv6_dip_care[3] = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv6.ipv6_dip_care[3] |= ((uint32) pEntry_table->care_field1 << 16);
                    pPredefined_entry->un.ipv6.ipv6_dip_care[2] = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv6.ipv6_dip_care[2] |= ((uint32) pEntry_table->care_field3 << 16);
                    pPredefined_entry->un.ipv6.ipv6_dip_care[1] = 0;
                    pPredefined_entry->un.ipv6.ipv6_dip_care[0] = 0;

                    pPredefined_entry->un.ipv6.next_header = pEntry_table->data_field4 & 0xff;
                    pPredefined_entry->un.ipv6.next_header_care = pEntry_table->care_field4 & 0xff;

                    pPredefined_entry->un.ipv6.dscp = (pEntry_table->data_field4 >> 8) & 0xff;
                    pPredefined_entry->un.ipv6.dscp_care = (pEntry_table->care_field4 >> 8) & 0xff;

                    pPredefined_entry->un.ipv6.flow_label = pEntry_table->data_field5 & 0xffff;
                    pPredefined_entry->un.ipv6.flow_label |= (((uint32) pEntry_table->data_field6 & 0xf) << 16);
                    pPredefined_entry->un.ipv6.flow_label_care = pEntry_table->care_field5 & 0xffff;
                    pPredefined_entry->un.ipv6.flow_label_care |= (((uint32) pEntry_table->care_field6 & 0xf) << 16);
                    break;
           
                case L4FMT_TCP:
                    /* IPv6 TCP */
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[3] = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[3] |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[2] = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[2] |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[1] = 0;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip[0] = 0;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[3] = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[3] |= ((uint32) pEntry_table->care_field1 << 16);
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[2] = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[2] |= ((uint32) pEntry_table->care_field3 << 16);
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[1] = 0;
                    pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[0] = 0;

                    pPredefined_entry->un.ipv6_tcp.src_port = pEntry_table->data_field5 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.src_port_care = pEntry_table->care_field5 & 0xffff;
                    
                    pPredefined_entry->un.ipv6_tcp.dst_port = pEntry_table->data_field4 & 0xffff;
                    pPredefined_entry->un.ipv6_tcp.dst_port_care = pEntry_table->care_field4 & 0xffff;

                    pPredefined_entry->un.ipv6_tcp.tcp_flag = pEntry_table->data_field6 & 0x3f;
                    pPredefined_entry->un.ipv6_tcp.tcp_flag_care = pEntry_table->care_field6 & 0x3f;
                    break;
                    
                case L4FMT_UDP:
                    /* IPv6 UDP */
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[3] = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[3] |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[2] = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[2] |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[1] = 0;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip[0] = 0;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[3] = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[3] |= ((uint32) pEntry_table->care_field1 << 16);
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[2] = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[2] |= ((uint32) pEntry_table->care_field3 << 16);
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[1] = 0;
                    pPredefined_entry->un.ipv6_udp.ipv6_dip_care[0] = 0;

                    pPredefined_entry->un.ipv6_udp.src_port = pEntry_table->data_field5 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.src_port_care = pEntry_table->care_field5 & 0xffff;

                    pPredefined_entry->un.ipv6_udp.dst_port = pEntry_table->data_field4 & 0xffff;
                    pPredefined_entry->un.ipv6_udp.dst_port_care = pEntry_table->care_field4 & 0xffff;
                    break;

                case L4FMT_ICMPV6:
                    /* ICMPv6 */
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[3] = pEntry_table->data_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[3] |= ((uint32) pEntry_table->data_field1 << 16);
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[2] = pEntry_table->data_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[2] |= ((uint32) pEntry_table->data_field3 << 16);
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[1] = 0;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip[0] = 0;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[3] = pEntry_table->care_field0 & 0xffff;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[3] |= ((uint32) pEntry_table->care_field1 << 16);
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[2] = pEntry_table->care_field2 & 0xffff;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[2] |= ((uint32) pEntry_table->care_field3 << 16);
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[1] = 0;
                    pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[0] = 0;

                    pPredefined_entry->un.ipv6_icmp.icmp_type = (pEntry_table->data_field5 >> 8) & 0xff;
                    pPredefined_entry->un.ipv6_icmp.icmp_type_care = (pEntry_table->care_field5 >> 8) & 0xff;

                    pPredefined_entry->un.ipv6_icmp.icmp_code = pEntry_table->data_field5 & 0xff;
                    pPredefined_entry->un.ipv6_icmp.icmp_code_care = pEntry_table->care_field5 & 0xff;
                    break;

                default:
                    break;
            }                        
            break;

        default:
            break;
    }                                                
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_piePreDefinedRuleEntry_get */

/* Function Name:
 *      dal_esw_pie_piePreDefinedRuleEntry_set
 * Description:
 *      Set the pre-defined entry data to buffer.
 * Input:
 *      unit              - unit id
 *      pEntry_buffer     - data buffer of PIE entry  
 *      pPredefined_entry - pre-defined entry data
 * Output:
 *      None.  
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_piePreDefinedRuleEntry_set(
    uint32                          unit,
    uint8                           *pEntry_buffer,
    rtk_pie_preDefinedRuleEntry_t   *pPredefined_entry)
{
    uint32                  value;
    rtk_pie_entryTable_t    *pEntry_table = NULL;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pEntry_buffer=%x, pPredefined_entry=%x", 
                                          unit, pEntry_buffer, pPredefined_entry);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pPredefined_entry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pPredefined_entry->valid > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pPredefined_entry->lookup_phase > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pPredefined_entry->src_phy_port > DAL_ESW_PIE_BIT_VALUE_MAX(5)) ||
        (pPredefined_entry->tgl2fmt > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pPredefined_entry->extra_tag > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pPredefined_entry->tgl23fmt > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pPredefined_entry->tgl4fmt > DAL_ESW_PIE_BIT_VALUE_MAX(3)) ||
        (pPredefined_entry->pppoe > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pPredefined_entry->ttl_type > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pPredefined_entry->dst_vid > DAL_ESW_PIE_BIT_VALUE_MAX(12)) ||
        (pPredefined_entry->src_vid > DAL_ESW_PIE_BIT_VALUE_MAX(12)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Entry value error");
        return RT_ERR_FAILED;
    }

    pEntry_table = (rtk_pie_entryTable_t *) pEntry_buffer;

    pEntry_table->valid = pPredefined_entry->valid & 0x1;
    pEntry_table->data_field8 &= 0x3fff;
    pEntry_table->data_field8 |= (pPredefined_entry->lookup_phase & 0x3) << 14;
    pEntry_table->care_field8 &= 0x3fff;
    pEntry_table->care_field8 |= (pPredefined_entry->lookup_phase_care & 0x3) << 14;

    pEntry_table->data_field8 &= 0xc1ff;
    pEntry_table->data_field8 |= (pPredefined_entry->src_phy_port & 0x1f) << 9;
    pEntry_table->care_field8 &= 0xc1ff;
    pEntry_table->care_field8 |= (pPredefined_entry->src_phy_port_care & 0x1f) << 9;

    switch (pPredefined_entry->tgl2fmt)
    {
        case L2FMT_NO_TAG:
            value = 0;
            break;

        case L2FMT_INNER_TAG:
            value = 1;
            break;

        case L2FMT_OUTER_TAG:
            value = 2;
            break;

        case L2FMT_BOTH_INNER_OUTER_TAG:
            value = 3;
            break;

        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TGL2FMT value error");
            return RT_ERR_FAILED;
    }
    pEntry_table->data_field8 &= 0xfe7f;
    pEntry_table->data_field8 |= (value & 0x3) << 7;
    pEntry_table->care_field8 &= 0xfe7f;
    pEntry_table->care_field8 |= (pPredefined_entry->tgl2fmt_care & 0x3) << 7;

    pEntry_table->data_field8 &= 0xffbf;
    pEntry_table->data_field8 |= (pPredefined_entry->extra_tag & 0x1) << 6;
    pEntry_table->care_field8 &= 0xffbf;
    pEntry_table->care_field8 |= (pPredefined_entry->extra_tag_care & 0x1) << 6;


    switch (pPredefined_entry->tgl23fmt)
    {
        case L23FMT_ARP:
            value = 0;
            break;

        case L23FMT_L2:
            value = 1;
            break;

        case L23FMT_IPV4:
            value = 2;
            break;

        case L23FMT_IPV6:
            value = 3;
            break;

        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TGL23FMT value error");
            return RT_ERR_FAILED;
    }
    pEntry_table->data_field8 &= 0xffcf;
    pEntry_table->data_field8 |= (value & 0x3) << 4;
    pEntry_table->care_field8 &= 0xffcf;
    pEntry_table->care_field8 |= (pPredefined_entry->tgl23fmt_care & 0x3) << 4;

    switch (pPredefined_entry->tgl4fmt)
    {
        case L4FMT_UNKNOWN:
            value = 0;
            break;

        case L4FMT_TCP:
            value = 1;
            break;

        case L4FMT_UDP:
            value = 2;
            break;

        case L4FMT_ICMP:
            value = 3;
            break;

        case L4FMT_IGMP:
            value = 4;
            break;

        case L4FMT_ICMPV6:
            value = 5;
            break;

        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TGL4FMT value error");
            return RT_ERR_FAILED;
    }
    pEntry_table->data_field8 &= 0xfff1;
    pEntry_table->data_field8 |= (value & 0x7) << 1;
    pEntry_table->care_field8 &= 0xfff1;
    pEntry_table->care_field8 |= (pPredefined_entry->tgl4fmt_care & 0x7) << 1;

    pEntry_table->data_field8 &= 0xfffe;
    pEntry_table->data_field8 |= pPredefined_entry->pppoe & 0x1;
    pEntry_table->care_field8 &= 0xfffe;
    pEntry_table->care_field8 |= pPredefined_entry->pppoe_care & 0x1;

    pEntry_table->data_field6 &= 0xff3f;
    pEntry_table->data_field6 |= (pPredefined_entry->ttl_type & 0x3) << 6;
    pEntry_table->care_field6 &= 0xff3f;
    pEntry_table->care_field6 |= (pPredefined_entry->ttl_type_care & 0x3) << 6;

    pEntry_table->data_field6 &= 0x00ff;
    pEntry_table->data_field6 |= (pPredefined_entry->dst_vid & 0xff) << 8;
    pEntry_table->care_field6 &= 0x00ff;
    pEntry_table->care_field6 |= (pPredefined_entry->dst_vid_care & 0xff) << 8;

    pEntry_table->data_field7 &= 0xfff0;
    pEntry_table->data_field7 |= (pPredefined_entry->dst_vid >> 8) & 0xf;
    pEntry_table->care_field7 &= 0xfff0;
    pEntry_table->care_field7 |= (pPredefined_entry->dst_vid_care >> 8) & 0xf;

    pEntry_table->data_field7 &= 0x000f;
    pEntry_table->data_field7 |= (pPredefined_entry->src_vid & 0x0fff) << 4;
    pEntry_table->care_field7 &= 0x000f;
    pEntry_table->care_field7 |= (pPredefined_entry->src_vid_care & 0x0fff) << 4;

    switch (pPredefined_entry->tgl23fmt)
    {
        case L23FMT_ARP:
            if (L4FMT_UNKNOWN == pPredefined_entry->tgl4fmt)
            {/* ARP */
                pEntry_table->data_field0 = pPredefined_entry->un.arp.sip & 0xffff;
                pEntry_table->care_field0 = pPredefined_entry->un.arp.sip_care & 0xffff;
                pEntry_table->data_field1 = (pPredefined_entry->un.arp.sip >> 16) & 0xffff;
                pEntry_table->care_field1 = (pPredefined_entry->un.arp.sip_care >> 16) & 0xffff;

                pEntry_table->data_field2 = pPredefined_entry->un.arp.dip & 0xffff;
                pEntry_table->care_field2 = pPredefined_entry->un.arp.dip_care & 0xffff;
                pEntry_table->data_field3 = (pPredefined_entry->un.arp.dip >> 16) & 0xffff;
                pEntry_table->care_field3 = (pPredefined_entry->un.arp.dip_care >> 16) & 0xffff;
            }
            break;

        case L23FMT_IPV4:
            switch (pPredefined_entry->tgl4fmt)
            {
                case L4FMT_UNKNOWN:
                    /* IPv4 */
                    pEntry_table->data_field0 = pPredefined_entry->un.ipv4.sip & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv4.sip_care & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv4.sip >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv4.sip_care >> 16) & 0xffff;

                    pEntry_table->data_field2 = pPredefined_entry->un.ipv4.dip & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv4.dip_care & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv4.dip >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv4.dip_care >> 16) & 0xffff;

                    pEntry_table->data_field5 &= 0x00ff;
                    pEntry_table->data_field5 |= (pPredefined_entry->un.ipv4.protocol & 0xff) << 8;
                    pEntry_table->care_field5 &= 0x00ff;
                    pEntry_table->care_field5 |= (pPredefined_entry->un.ipv4.protocol_care & 0xff) << 8;

                    pEntry_table->data_field5 &= 0xff00;
                    pEntry_table->data_field5 |= pPredefined_entry->un.ipv4.tos & 0xff;
                    pEntry_table->care_field5 &= 0xff00;
                    pEntry_table->care_field5 |= pPredefined_entry->un.ipv4.tos_care & 0xff;

                    pEntry_table->data_field4 &= 0x00ff;
                    pEntry_table->data_field4 |= (pPredefined_entry->un.ipv4.ttl & 0xff) << 8;
                    pEntry_table->care_field4 &= 0x00ff;
                    pEntry_table->care_field4 |= (pPredefined_entry->un.ipv4.ttl_care & 0xff) << 8;
                    break;

                case L4FMT_TCP:
                    /* IPv4 TCP */
                    /* input value range check */
                    if ((pPredefined_entry->un.ipv4_tcp.tcp_flag > DAL_ESW_PIE_BIT_VALUE_MAX(6)))
                    {
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TCP flag value error");
                        return RT_ERR_FAILED;
                    }

                    pEntry_table->data_field0 = pPredefined_entry->un.ipv4_tcp.sip & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv4_tcp.sip_care & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv4_tcp.sip >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv4_tcp.sip_care >> 16) & 0xffff;

                    pEntry_table->data_field2 = pPredefined_entry->un.ipv4_tcp.dip & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv4_tcp.dip_care & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv4_tcp.dip >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv4_tcp.dip_care >> 16) & 0xffff;

                    pEntry_table->data_field5 = pPredefined_entry->un.ipv4_tcp.src_port & 0xffff;
                    pEntry_table->care_field5 = pPredefined_entry->un.ipv4_tcp.src_port_care & 0xffff;

                    pEntry_table->data_field4 = pPredefined_entry->un.ipv4_tcp.dst_port & 0xffff;
                    pEntry_table->care_field4 = pPredefined_entry->un.ipv4_tcp.dst_port_care & 0xffff;

                    pEntry_table->data_field6 &= 0xffc0;
                    pEntry_table->data_field6 |= (pPredefined_entry->un.ipv4_tcp.tcp_flag & 0x3f);
                    pEntry_table->care_field6 &= 0xffc0;
                    pEntry_table->care_field6 |= (pPredefined_entry->un.ipv4_tcp.tcp_flag_care & 0x3f);
                    break;

                case L4FMT_UDP:
                    /* IPv4 UDP */
                    pEntry_table->data_field0 = pPredefined_entry->un.ipv4_udp.sip & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv4_udp.sip_care & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv4_udp.sip >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv4_udp.sip_care >> 16) & 0xffff;

                    pEntry_table->data_field2 = pPredefined_entry->un.ipv4_udp.dip & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv4_udp.dip_care & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv4_udp.dip >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv4_udp.dip_care >> 16) & 0xffff;

                    pEntry_table->data_field5 = pPredefined_entry->un.ipv4_udp.src_port & 0xffff;
                    pEntry_table->care_field5 = pPredefined_entry->un.ipv4_udp.src_port_care & 0xffff;

                    pEntry_table->data_field4 = pPredefined_entry->un.ipv4_udp.dst_port & 0xffff;
                    pEntry_table->care_field4 = pPredefined_entry->un.ipv4_udp.dst_port_care & 0xffff;
                    break;

                case L4FMT_ICMP:
                    pEntry_table->data_field0 = pPredefined_entry->un.ipv4_icmp.sip & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv4_icmp.sip_care & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv4_icmp.sip >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv4_icmp.sip_care >> 16) & 0xffff;

                    pEntry_table->data_field2 = pPredefined_entry->un.ipv4_icmp.dip & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv4_icmp.dip_care & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv4_icmp.dip >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv4_icmp.dip_care >> 16) & 0xffff;

                    pEntry_table->data_field5 &= 0x00ff;
                    pEntry_table->data_field5 |= (pPredefined_entry->un.ipv4_icmp.icmp_type & 0xff) << 8;
                    pEntry_table->care_field5 &= 0x00ff;
                    pEntry_table->care_field5 |= (pPredefined_entry->un.ipv4_icmp.icmp_type_care & 0xff) << 8;

                    pEntry_table->data_field5 &= 0xff00;
                    pEntry_table->data_field5 |= (pPredefined_entry->un.ipv4_icmp.icmp_code & 0xff);
                    pEntry_table->care_field5 &= 0xff00;
                    pEntry_table->care_field5 |= (pPredefined_entry->un.ipv4_icmp.icmp_code_care & 0xff);
                    break;

                case L4FMT_IGMP:
                    /* IPv4 IGMP */
                    /* input value range check */
                    if ((pPredefined_entry->un.ipv4_igmp.group_ip > DAL_ESW_PIE_BIT_VALUE_MAX(28)))
                    {
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Group IP value error");
                        return RT_ERR_FAILED;
                    }

                    pEntry_table->data_field0 = pPredefined_entry->un.ipv4_igmp.sip & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv4_igmp.sip_care & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv4_igmp.sip >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv4_igmp.sip_care >> 16) & 0xffff;

                    pEntry_table->data_field2 = pPredefined_entry->un.ipv4_igmp.dip & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv4_igmp.dip_care & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv4_igmp.dip >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv4_igmp.dip_care >> 16) & 0xffff;

                    pEntry_table->data_field4 &= 0xff00;
                    pEntry_table->data_field4 |= (pPredefined_entry->un.ipv4_igmp.igmp_type & 0xff);
                    pEntry_table->care_field4 &= 0xff00;
                    pEntry_table->care_field4 |= (pPredefined_entry->un.ipv4_igmp.igmp_type_care & 0xff);

                    pEntry_table->data_field4 &= 0x00ff;
                    pEntry_table->data_field4 |= (pPredefined_entry->un.ipv4_igmp.group_ip & 0xff) << 8;
                    pEntry_table->care_field4 &= 0x00ff;
                    pEntry_table->care_field4 |= (pPredefined_entry->un.ipv4_igmp.group_ip_care & 0xff) << 8;
                    pEntry_table->data_field5 = (pPredefined_entry->un.ipv4_igmp.group_ip >> 8) & 0xffff;
                    pEntry_table->care_field5 = (pPredefined_entry->un.ipv4_igmp.group_ip_care >> 8) & 0xffff;
                    pEntry_table->data_field6 &= 0xfff0;
                    pEntry_table->data_field6 |= (pPredefined_entry->un.ipv4_igmp.group_ip >> 24) & 0xf;
                    pEntry_table->care_field6 &= 0xfff0;
                    pEntry_table->care_field6 |= (pPredefined_entry->un.ipv4_igmp.group_ip_care >> 24) & 0xf;
                    break;

                default:
                    break;
            }            
            break;

        case L23FMT_IPV6:
            switch (pPredefined_entry->tgl4fmt)
            {
                case L4FMT_UNKNOWN:
                    /* IPv6 */
                    /* input value range check */
                    if ((pPredefined_entry->un.ipv6.flow_label > DAL_ESW_PIE_BIT_VALUE_MAX(20)))
                    {
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Flow label value error");
                        return RT_ERR_FAILED;
                    }

                    pEntry_table->data_field0 = pPredefined_entry->un.ipv6.ipv6_dip[3] & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv6.ipv6_dip_care[3] & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv6.ipv6_dip[3] >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv6.ipv6_dip_care[3] >> 16) & 0xffff;
                    pEntry_table->data_field2 = pPredefined_entry->un.ipv6.ipv6_dip[2] & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv6.ipv6_dip_care[2] & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv6.ipv6_dip[2] >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv6.ipv6_dip_care[2] >> 16) & 0xffff;

                    pEntry_table->data_field4 &= 0xff00;
                    pEntry_table->data_field4 |= (pPredefined_entry->un.ipv6.next_header & 0xff);
                    pEntry_table->care_field4 &= 0xff00;
                    pEntry_table->care_field4 |= (pPredefined_entry->un.ipv6.next_header_care & 0xff);

                    pEntry_table->data_field4 &= 0x00ff;
                    pEntry_table->data_field4 |= (pPredefined_entry->un.ipv6.dscp & 0xff) << 8;
                    pEntry_table->care_field4 &= 0x00ff;
                    pEntry_table->care_field4 |= (pPredefined_entry->un.ipv6.dscp_care & 0xff) << 8;

                    pEntry_table->data_field5 = pPredefined_entry->un.ipv6.flow_label & 0xffff;
                    pEntry_table->care_field5 = pPredefined_entry->un.ipv6.flow_label_care & 0xffff;
                    pEntry_table->data_field6 &= 0xfff0;
                    pEntry_table->data_field6 |= (pPredefined_entry->un.ipv6.flow_label >> 16) & 0xf;
                    pEntry_table->care_field6 &= 0xfff0;
                    pEntry_table->care_field6 |= (pPredefined_entry->un.ipv6.flow_label_care >> 16) & 0xf;
                    break;

                case L4FMT_TCP:
                    /* IPv6 TCP */
                    /* input value range check */
                    if ((pPredefined_entry->un.ipv6_tcp.tcp_flag > DAL_ESW_PIE_BIT_VALUE_MAX(6)))
                    {
                        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "TCP flag value error");
                        return RT_ERR_FAILED;
                    }

                    pEntry_table->data_field0 = pPredefined_entry->un.ipv6_tcp.ipv6_dip[3] & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[3] & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv6_tcp.ipv6_dip[3] >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[3] >> 16) & 0xffff;
                    pEntry_table->data_field2 = pPredefined_entry->un.ipv6_tcp.ipv6_dip[2] & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[2] & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv6_tcp.ipv6_dip[2] >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv6_tcp.ipv6_dip_care[2] >> 16) & 0xffff;

                    pEntry_table->data_field5 = pPredefined_entry->un.ipv6_tcp.src_port & 0xffff;
                    pEntry_table->care_field5 = pPredefined_entry->un.ipv6_tcp.src_port_care & 0xffff;

                    pEntry_table->data_field4 = pPredefined_entry->un.ipv6_tcp.dst_port & 0xffff;
                    pEntry_table->care_field4 = pPredefined_entry->un.ipv6_tcp.dst_port_care & 0xffff;

                    pEntry_table->data_field6 &= 0xffc0;
                    pEntry_table->data_field6 |= (pPredefined_entry->un.ipv6_tcp.tcp_flag & 0x3f);
                    pEntry_table->care_field6 &= 0xffc0;
                    pEntry_table->care_field6 |= (pPredefined_entry->un.ipv6_tcp.tcp_flag_care & 0x3f);
                    break;

                case L4FMT_UDP:
                    /* IPv6 UDP */
                    pEntry_table->data_field0 = pPredefined_entry->un.ipv6_udp.ipv6_dip[3] & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv6_udp.ipv6_dip_care[3] & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv6_udp.ipv6_dip[3] >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv6_udp.ipv6_dip_care[3] >> 16) & 0xffff;
                    pEntry_table->data_field2 = pPredefined_entry->un.ipv6_udp.ipv6_dip[2] & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv6_udp.ipv6_dip_care[2] & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv6_udp.ipv6_dip[2] >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv6_udp.ipv6_dip_care[2] >> 16) & 0xffff;

                    pEntry_table->data_field5 = pPredefined_entry->un.ipv6_udp.src_port & 0xffff;
                    pEntry_table->care_field5 = pPredefined_entry->un.ipv6_udp.src_port_care & 0xffff;

                    pEntry_table->data_field4 = pPredefined_entry->un.ipv6_udp.dst_port & 0xffff;
                    pEntry_table->care_field4 = pPredefined_entry->un.ipv6_udp.dst_port_care & 0xffff;
                    break;

                case L4FMT_ICMPV6:
                    /* ICMPv6 */
                    pEntry_table->data_field0 = pPredefined_entry->un.ipv6_icmp.ipv6_dip[3] & 0xffff;
                    pEntry_table->care_field0 = pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[3] & 0xffff;
                    pEntry_table->data_field1 = (pPredefined_entry->un.ipv6_icmp.ipv6_dip[3] >> 16) & 0xffff;
                    pEntry_table->care_field1 = (pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[3] >> 16) & 0xffff;
                    pEntry_table->data_field2 = pPredefined_entry->un.ipv6_icmp.ipv6_dip[2] & 0xffff;
                    pEntry_table->care_field2 = pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[2] & 0xffff;
                    pEntry_table->data_field3 = (pPredefined_entry->un.ipv6_icmp.ipv6_dip[2] >> 16) & 0xffff;
                    pEntry_table->care_field3 = (pPredefined_entry->un.ipv6_icmp.ipv6_dip_care[2] >> 16) & 0xffff;

                    pEntry_table->data_field5 &= 0x00ff;
                    pEntry_table->data_field5 |= (pPredefined_entry->un.ipv6_icmp.icmp_type & 0xff) << 8;
                    pEntry_table->care_field5 &= 0x00ff;
                    pEntry_table->care_field5 |= (pPredefined_entry->un.ipv6_icmp.icmp_type_care & 0xff) << 8;
                    pEntry_table->data_field5 &= 0xff00;
                    pEntry_table->data_field5 |= pPredefined_entry->un.ipv6_icmp.icmp_code & 0xff;
                    pEntry_table->care_field5 &= 0xff00;
                    pEntry_table->care_field5 |= pPredefined_entry->un.ipv6_icmp.icmp_code_care & 0xff;
                    break;

                default:
                    break;
            }                        
            break;

        default:
            break;
    }                                                

    #if 0//(defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    /* for big endian, fix me */
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field0);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field0);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field1);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field1);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field2);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field2);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field3);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field3);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field4);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field4);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field5);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field5);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field6);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field6);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field7);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field7);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->data_field8);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->care_field8);
    DAL_ESW_PIE_BIG_ENDIAN16(pEntry_table->valid);
    #endif

    return RT_ERR_OK;
} /* end of dal_esw_pie_piePreDefinedRuleEntry_set */

/* Function Name:
 *      dal_esw_pie_pieRuleEntry_read
 * Description:
 *      Read the entry data from specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 * Output:
 *      pEntry_buffer - data buffer of PIE entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 */
int32
dal_esw_pie_pieRuleEntry_read(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    int32               ret;
    uint32              table_index;
    pie_rule_entry_t    pie_entry;
    #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    uint32 index;
    uint8  ednData;
    #endif    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, entry_idx=%d", unit, entry_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);
    
    osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));

    /*translate entry index to table index*/
    table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* get entry from chip */
    PIE_SEM_LOCK(unit);
    
    if ((ret = table_read(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    osal_memcpy(pEntry_buffer, &pie_entry, sizeof(pie_rule_entry_t)); /*need to make sure the data size of pie_entry is not large than pEntry_buffer*/

    #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    /* for big endian, fix me */
    for (index = 0; index < 10; index++)
    {
        ednData = *(pEntry_buffer + index*4);
        *(pEntry_buffer + index*4) = *(pEntry_buffer + index*4 + 3);
        *(pEntry_buffer + index*4 + 3) = ednData;
    
        ednData = *(pEntry_buffer + index*4 + 1);
        *(pEntry_buffer + index*4 + 1) = *(pEntry_buffer + index*4 + 2);
        *(pEntry_buffer + index*4 + 2) = ednData;
    }    
    #endif
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pEntry_buffer=%x", *pEntry_buffer);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntry_read */

/* Function Name:
 *      dal_esw_pie_pieRuleEntry_write
 * Description:
 *      Write the entry data to specified PIE entry.
 * Input:
 *      unit          - unit id
 *      entry_idx     - PIE entry index
 *      pEntry_buffer - data buffer of PIE entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The entry_idx is for logical view from 0 to (physical block number * physical block size - 1),
 *         driver will transfer it to physical entry index.
 */
int32
dal_esw_pie_pieRuleEntry_write(
    uint32              unit,
    rtk_pie_id_t        entry_idx,
    uint8               *pEntry_buffer)
{
    int32               ret;
    uint32              table_index;
    pie_rule_entry_t    pie_entry;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, entry_idx=%d, pEntry_buffer=%x", unit, entry_idx, pEntry_buffer);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pEntry_buffer), RT_ERR_NULL_POINTER);

    #if (defined(CONFIG_SDK_PIE_DATA_ENDIAN_LITTLE) && defined(CONFIG_SDK_ENDIAN_BIG)) || (defined(CONFIG_SDK_PIE_DATA_ENDIAN_BIG) && defined(CONFIG_SDK_ENDIAN_LITTLE))
    /* for big endian, fix me */
    {
        uint32 index;
        uint8  ednData;
        for (index = 0; index < 10; index++)
        {
            ednData = *(pEntry_buffer + index*4);
            *(pEntry_buffer + index*4) = *(pEntry_buffer + index*4 + 2);
            *(pEntry_buffer + index*4 + 2) = ednData;
        
            ednData = *(pEntry_buffer + index*4 + 1);
            *(pEntry_buffer + index*4 + 1) = *(pEntry_buffer + index*4 + 3);
            *(pEntry_buffer + index*4 + 3) = ednData;
        }    
    }
    #endif
    
    osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
    osal_memcpy(&pie_entry, pEntry_buffer, sizeof(pie_rule_entry_t));

    /*translate entry index to table index*/
    table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
    
    /* set entry to chip */
    PIE_SEM_LOCK(unit);
    
    if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleEntry_write */

/* Function Name:
 *      dal_esw_pie_pieRuleEntry_del
 * Description:
 *      Delete the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Clear the content of specified PIE entry to be 0.
 *      2. If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *         and then return to index 0 to continue to content.end.
 */
int32
dal_esw_pie_pieRuleEntry_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    /* Use software clear */
    int32           ret;
    uint32          entry_idx, max_idx;
    uint32          table_index;
    pie_rule_entry_t    pie_entry;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->start_idx >= max_idx) || (pContent->end_idx >= max_idx))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    if (pContent->start_idx <= pContent->end_idx)
    {
        for (entry_idx = pContent->start_idx; entry_idx <= pContent->end_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }
    else
    {
        for (entry_idx = pContent->end_idx; entry_idx < max_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
        for (entry_idx = 0; entry_idx <= pContent->start_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    /* Use chip clear */
    int32           ret;
    uint32          table_index;
    uint32          value;
    cbccr_entry_t   pie_entry_clear;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->start_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->end_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_clear, 0, sizeof(cbccr_entry_t));

    /*translate action index to table index*/
    table_index = 0;

    PIE_SEM_LOCK(unit);
    
    /* set CLR_ACTTBL */
    value = 0;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_ACTTBLf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set CLR_PIE */
    value = 1;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_PIEf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_START */
    value = pContent->start_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_STARTf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_END */
    value = pContent->end_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_ENDf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_STOPALEf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_CBCCRt, table_index, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntry_del */

/* Function Name:
 *      dal_esw_pie_pieRuleEntry_move
 * Description:
 *      Move the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
int32
dal_esw_pie_pieRuleEntry_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    int32               ret;
    uint32              i, entry_idx, max_idx;
    uint32              table_from_index, table_to_index;
    pie_rule_entry_t    pie_entry;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) ||
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from + pContent->entry_length - 1;
            new_move_to = pContent->move_to + pContent->entry_length - 1;
            if (new_move_to >= max_idx)
            {
                new_move_length = max_idx - pContent->move_to;
                new_move_from = pContent->move_from + new_move_length - 1;
                new_move_to = pContent->move_to + new_move_length - 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from >= max_idx)
            {
                new_move_length = max_idx - pContent->move_from;
            }
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from < pContent->entry_length)
            {
                new_move_length = new_move_from + 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            if (pContent->move_to < pContent->entry_length)
            {
                new_move_length = pContent->move_to + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
                new_move_to = 0;
            }
            else
            {
                new_move_to = pContent->move_to - new_move_length + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
            }
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from + i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            entry_idx = new_move_to + i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from - i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            entry_idx = new_move_to - i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_entry_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 0;/* 0 for move*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 1;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 0;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default: 
            PIE_SEM_UNLOCK(unit);
            return RT_ERR_OUT_OF_RANGE;
    }

    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntry_move */

/* Function Name:
 *      dal_esw_pie_pieRuleEntry_swap
 * Description:
 *      Swap the specified PIE entry.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieRuleEntry_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    int32               ret;
    uint32              i, entry_idx, max_idx;
    uint32              table_from_index, table_to_index;
    pie_rule_entry_t    pie_from_entry, pie_to_entry;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) ||
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    new_move_direction = pContent->direction;
    new_move_length = pContent->entry_length;
    new_move_from = pContent->move_from;
    new_move_to = pContent->move_to;
    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            if ((pContent->move_from + new_move_length) >= pContent->move_to)
                new_move_length = pContent->move_to - pContent->move_from;
            if ((pContent->move_to + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_to;
        }
        else
        {
            if ((pContent->move_from + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_from;
            if ((pContent->move_to + new_move_length) >= pContent->move_from)
                new_move_length = pContent->move_from - pContent->move_to;
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            if (new_move_from < new_move_length)
                new_move_length = new_move_from + 1;
            if ((new_move_to - new_move_length + 1) <= new_move_from)
                new_move_length = new_move_to - new_move_from;
        }
        else
        {
            if (pContent->move_to < new_move_length)
                new_move_length = pContent->move_to + 1;
            if ((new_move_from - new_move_length + 1) <= new_move_to)
                new_move_length = new_move_from - new_move_to;
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from + i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            entry_idx = new_move_to + i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from - i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            entry_idx = new_move_to - i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_entry_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 1;/* 1 for swap*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 1;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 0;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default: 
            PIE_SEM_UNLOCK(unit);
            return RT_ERR_OUT_OF_RANGE;
    }
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntry_swap */


/* Function Name:
 *      dal_esw_pie_pieRuleEntryAction_del
 * Description:
 *      Delete the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE entry information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Clear the content of specified PIE entry and action to be 0.
 *      2. If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry and action
 *         and then return to index 0 to continue to content.end.
 */
int32
dal_esw_pie_pieRuleEntryAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    /* Use software clear */
    int32           ret;
    uint32          entry_idx, max_idx;
    uint32          table_index;
    pie_rule_entry_t    pie_entry;
    pie_act_entry_t     pie_action;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->start_idx >= max_idx) || (pContent->end_idx >= max_idx))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    if (pContent->start_idx <= pContent->end_idx)
    {
        for (entry_idx = pContent->start_idx; entry_idx <= pContent->end_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }
    else
    {
        for (entry_idx = pContent->end_idx; entry_idx < max_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
        for (entry_idx = 0; entry_idx <= pContent->start_idx; entry_idx++)
        {
            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            /*translate entry index to table index*/
            table_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }

            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    /* Use chip clear */
    int32           ret;
    uint32          table_index;
    uint32          value;
    cbccr_entry_t   pie_entry_clear;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->start_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->end_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_clear, 0, sizeof(cbccr_entry_t));

    /*translate action index to table index*/
    table_index = 0;

    PIE_SEM_LOCK(unit);
    
    /* set CLR_ACTTBL */
    value = 1;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_ACTTBLf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set CLR_PIE */
    value = 1;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_PIEf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_START */
    value = pContent->start_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_STARTf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_END */
    value = pContent->end_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_ENDf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_STOPALEf, &value, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_CBCCRt, table_index, (uint32 *) &pie_entry_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntryAction_del */

/* Function Name:
 *      dal_esw_pie_pieRuleEntryAction_move
 * Description:
 *      Move the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry and action,
 *      then ASIC stop action.
 */
int32
dal_esw_pie_pieRuleEntryAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    uint64              counter64 = 0;
    int32               ret;
    uint32              i, entry_idx, max_idx;
    uint32              stats_type, counter32, value;
    uint32              table_from_index, table_to_index, table_index;
    pie_rule_entry_t    pie_entry;
    pie_act_entry_t     pie_action;
    log_entry_t         pie_counter;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) ||
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from + pContent->entry_length - 1;
            new_move_to = pContent->move_to + pContent->entry_length - 1;
            if (new_move_to >= max_idx)
            {
                new_move_length = max_idx - pContent->move_to;
                new_move_from = pContent->move_from + new_move_length - 1;
                new_move_to = pContent->move_to + new_move_length - 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from >= max_idx)
            {
                new_move_length = max_idx - pContent->move_from;
            }
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from < pContent->entry_length)
            {
                new_move_length = new_move_from + 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            if (pContent->move_to < pContent->entry_length)
            {
                new_move_length = pContent->move_to + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
                new_move_to = 0;
            }
            else
            {
                new_move_to = pContent->move_to - new_move_length + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
            }
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from + i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_type, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            entry_idx = new_move_to + i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            osal_memset(&pie_counter, 0, sizeof(log_entry_t));
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from - i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_type, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            entry_idx = new_move_to - i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            osal_memset(&pie_entry, 0, sizeof(pie_rule_entry_t));
            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            osal_memset(&pie_counter, 0, sizeof(log_entry_t));
            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_entry_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 0;/* 0 for move*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 1;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 1;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default: 
            PIE_SEM_UNLOCK(unit);
            return RT_ERR_OUT_OF_RANGE;
    }

    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntryAction_move */

/* Function Name:
 *      dal_esw_pie_pieRuleEntryAction_swap
 * Description:
 *      Swap the specified PIE entry and action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE entry and action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieRuleEntryAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    uint64              from_counter64 = 0, to_counter64 = 0;
    int32               ret;
    uint32              i, entry_idx, max_idx;
    uint32              stats_from_type, stats_to_type, from_counter32, to_counter32, value;
    uint32              table_from_index, table_to_index, table_index;
    pie_rule_entry_t    pie_from_entry, pie_to_entry;
    pie_act_entry_t     pie_from_action, pie_to_action;
    log_entry_t         pie_from_counter, pie_to_counter;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_FILTER_ID(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) ||
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    new_move_direction = pContent->direction;
    new_move_length = pContent->entry_length;
    new_move_from = pContent->move_from;
    new_move_to = pContent->move_to;
    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            if ((pContent->move_from + new_move_length) >= pContent->move_to)
                new_move_length = pContent->move_to - pContent->move_from;
            if ((pContent->move_to + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_to;
        }
        else
        {
            if ((pContent->move_from + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_from;
            if ((pContent->move_to + new_move_length) >= pContent->move_from)
                new_move_length = pContent->move_from - pContent->move_to;
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            if (new_move_from < new_move_length)
                new_move_length = new_move_from + 1;
            if ((new_move_to - new_move_length + 1) <= new_move_from)
                new_move_length = new_move_to - new_move_from;
        }
        else
        {
            if (pContent->move_to < new_move_length)
                new_move_length = pContent->move_to + 1;
            if ((new_move_from - new_move_length + 1) <= new_move_to)
                new_move_length = new_move_from - new_move_to;
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from + i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_from_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_from_type, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            entry_idx = new_move_to + i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_to_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_to_type, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_to_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_to_counter, 0, sizeof(log_entry_t));
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = from_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((from_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_from_counter, 0, sizeof(log_entry_t));
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = to_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((to_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            entry_idx = new_move_from - i;
            table_from_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_from_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_from_type, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            entry_idx = new_move_to - i;
            table_to_index = ((entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)) << 6) + (entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit));
            if ((ret = table_read(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_read(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_to_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_to_type, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_to_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_PIEt, table_to_index, (uint32 *) &pie_from_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_to_counter, 0, sizeof(log_entry_t));
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = from_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((from_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_PIEt, table_from_index, (uint32 *) &pie_to_entry)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_from_counter, 0, sizeof(log_entry_t));
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = to_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((to_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_entry_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) ||
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_entry_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 1;/* 1 for swap*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 1;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 1;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default: 
            PIE_SEM_UNLOCK(unit);
            return RT_ERR_OUT_OF_RANGE;
    }
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_entry_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleEntryAction_swap */


static int32 dal_esw_pie_pieActionInfoField_get(uint32 unit,pie_act_entry_t *pPie_action, uint32 action_info_field, uint32 *value)
{
    int32   ret;
    uint32  field_idx;
    
    switch (action_info_field)
    {
        case 1:
            field_idx = ESW_ACTTBL_AIF1f;
            break;
        case 2:
            field_idx = ESW_ACTTBL_AIF2f;
            break;            
        case 3:
            field_idx = ESW_ACTTBL_AIF3f;
            break;
        case 4:
            field_idx = ESW_ACTTBL_AIF4f;
            break;
        case 5:
            field_idx = ESW_ACTTBL_AIF5f;
            break;
        default:
            return RT_ERR_OUT_OF_RANGE;
    }

    if ((ret = table_field_get( unit, ESW_ACTTBLt, field_idx, value, (uint32 *) pPie_action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
            
    return RT_ERR_OK;    
}/* end of dal_esw_pie_pieActionInfoField_get */

/* Function Name:
 *      dal_esw_pie_pieRuleAction_get
 * Description:
 *      Get the PIE action configuration from specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 * Output:
 *      pAction    - PIE action configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_pieRuleAction_get(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    int32           ret;
    uint32          table_index;
    uint32          value;
    uint32          action_info_field;
    pie_act_entry_t pie_action;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, action_idx=%d", unit, action_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((action_idx >= HAL_MAX_NUM_OF_PIE_ACTION(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    
    osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));

    /*translate action index to table index*/
    table_index = action_idx;
    
    /* get entry from chip */
    PIE_SEM_LOCK(unit);
    
    if ((ret = table_read(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* get HITIND */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_HITINDf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->hit_indication = DISABLED;
            break;
        case 1:
            pAction->hit_indication = ENABLED;
            break;            
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get STATISTICS */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->statistics = COUNT_NONE;
            break;
        case 1:
            pAction->statistics = COUNT_PACKET;
            break;            
        case 2:
            pAction->statistics = COUNT_BYTE32;
            break;       
        case 3:
            pAction->statistics = COUNT_BYTE64;
            break;       
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get CP2CPU */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_CP2CPUf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->cp2cpu = DISABLED;
            break;
        case 1:
            pAction->cp2cpu = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get DROP */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_DROPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->drop = DROP_PERMIT;
            break;
        case 1:
            pAction->drop = DROP_DROP;
            break;            
        case 2:
            pAction->drop = DROP_WITHDRAW_DROP;
            break;       
        case 3:
            pAction->drop = DROP_RESERVED;
            break;          
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get OUTTAGOP */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_OUTTAGOPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->outer_tag_op = DISABLED;
            break;
        case 1:
            pAction->outer_tag_op = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get REDIR */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_REDIRf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->redirect = DISABLED;
            break;
        case 1:
            pAction->redirect = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get INTAGOP */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_INTAGOPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->inner_tag_op = DISABLED;
            break;
        case 1:
            pAction->inner_tag_op = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get PRI */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_PRIf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->priority = DISABLED;
            break;
        case 1:
            pAction->priority = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get DSCP */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_DSCPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->dscp_remark_spid = DISABLED;
            break;
        case 1:
            pAction->dscp_remark_spid = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get POLICE_OUTRMK */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_POLICE_OUTRMKf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->police_outer_pri_remark = DISABLED;
            break;
        case 1:
            pAction->police_outer_pri_remark = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    /* get MIRROR */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_MIRRORf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pAction->mirror = DISABLED;
            break;
        case 1:
            pAction->mirror = ENABLED;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    action_info_field = 0;
    if ((ENABLED == pAction->outer_tag_op) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            pAction->outer_tag_op_field.withdraw = (value >> 17) & 0x1;
            pAction->outer_tag_op_field.outer_vid_ctrl = (value >> 15) & 0x3;
            pAction->outer_tag_op_field.outer_vid_info = (value >> 3) & 0xfff;
            pAction->outer_tag_op_field.outer_tag_op = value & 0x3; 
        }               
    }

    if ((ENABLED == pAction->redirect) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            pAction->redirect_field.withdraw = (value >> 17) & 0x1;
            switch ((value >> 15) & 0x3)
            {
                case 0:
                    pAction->redirect_field.opcode = REDIRECT_UNI_REDIRECT;
                    break;
                case 1:
                    pAction->redirect_field.opcode = REDIRECT_MULTI_REDIRECT;
                    break;
                case 2:
                    pAction->redirect_field.opcode = REDIRECT_UNI_ROUTE;
                    break;
                case 3:
                    pAction->redirect_field.opcode = REDIRECT_MULTI_ROUTE;
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                    return RT_ERR_FAILED;
            }    
        
            switch (pAction->redirect_field.opcode)
            {
                case REDIRECT_UNI_REDIRECT:
                    pAction->redirect_field.un.uniRedirect.cpu_tag = (value >> 5) & 0x1;
                    pAction->redirect_field.un.uniRedirect.dpn = value & 0x1f;
                    break;
                case REDIRECT_MULTI_REDIRECT:
                    pAction->redirect_field.un.multiRedirect.fwd_idx = value & 0x3ff;
                    break;                
                case REDIRECT_UNI_ROUTE:
                case REDIRECT_MULTI_ROUTE:    
                    pAction->redirect_field.un.route.ttl_dec = (value >> 14) & 0x1;
                    pAction->redirect_field.un.route.lookup_idx = value & 0x3fff;
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                    return RT_ERR_FAILED;
            }
        }
    }

    if ((ENABLED == pAction->inner_tag_op) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            switch (action_info_field)
            {
                case 1:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_INNER_VLAN_TAG_SIZE);
                    break;
                case 2:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_INNER_VLAN_TAG_SIZE);
                    break;
                case 3:
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                    return RT_ERR_FAILED;
            }
            
            pAction->inner_tag_op_field.withdraw = (value >> 16) & 0x1;
            pAction->inner_tag_op_field.inner_vid_ctrl = (value >> 14) & 0x3;
            pAction->inner_tag_op_field.inner_vid_info = (value >> 2) & 0xfff;
            pAction->inner_tag_op_field.inner_tag_op = value & 0x3;
        }
    }

    if ((ENABLED == pAction->priority) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            switch (action_info_field)
            {
                case 1:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                    break;
                case 2:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                    break;
                case 3:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                    break;
                case 4:
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                    return RT_ERR_FAILED;
            }

            pAction->priority_field.withdraw = (value >> 10) & 0x1;
            pAction->priority_field.sel_pri_dp = (value >> 7) & 0x7;
            pAction->priority_field.assign_pri = (value >> 6) & 0x1;
            pAction->priority_field.acl_pri = (value >> 3) & 0x7;
            pAction->priority_field.assign_dp = (value >> 2) & 0x1;
            pAction->priority_field.acl_dp = value & 0x3;
        }
    }

    if ((ENABLED == pAction->dscp_remark_spid) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            switch (action_info_field)
            {
                case 1:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                    break;
                case 2:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                    break;
                case 3:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                    break;
                case 4:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                    break;
                case 5:
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                    return RT_ERR_FAILED;
            }

            pAction->dscp_remark_spid_field.withdraw = (value >> 8) & 0x1;
            pAction->dscp_remark_spid_field.un.dscp_remark.un.data = value & 0xff;
        }
    }

    if ((ENABLED == pAction->police_outer_pri_remark) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            switch (action_info_field)
            {
                case 1:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                    break;
                case 2:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                    break;
                case 3:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                    break;
                case 4:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                    break;
                case 5:
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                    return RT_ERR_FAILED;
            }

            pAction->police_outer_pri_remark_field.withdraw = (value >> 8) & 0x1;
            pAction->police_outer_pri_remark_field.un.policer_idx = value & 0xff;
        }
    }
    
    if ((ENABLED == pAction->mirror) && (action_info_field < DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        action_info_field++;
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_get(unit, &pie_action, action_info_field, &value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
        else
        {
            switch (action_info_field)
            {
                case 1:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                    break;
                case 2:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                    break;
                case 3:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                    break;
                case 4:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                    break;
                case 5:
                    value = value >> (DAL_ESW_ACTION_INFO_FIELD5_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                    break;
                default:
                    PIE_SEM_UNLOCK(unit);
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                    return RT_ERR_FAILED;
            }

            pAction->mirror_field.withdraw = (value >> 2) & 0x1;
            pAction->mirror_field.mirror_idx = value & 0x3;
        }
    }
    
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pAction=%x", *pAction);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleAction_get */

/*
 * Counter index 0 and 64 can't mix 32bits and 64bits mode 
 */
static int32 dal_esw_pie_pieActionCheckMixedCounterType(uint32  unit, rtk_pie_id_t  action_idx, rtk_pie_statisticType_t type)
{
    int32           ret;
    uint32          table_index;
    uint32          value;
    pie_act_entry_t pie_action;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, action_idx=%d", unit, action_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /*Out of counter range, needn't to check*/
    RT_PARAM_CHK((action_idx >= HAL_MAX_NUM_OF_PIE_COUNTER(unit)), RT_ERR_OK);

    /* Check arguments */
    RT_PARAM_CHK((action_idx >= HAL_MAX_NUM_OF_PIE_ACTION(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((type >= PIE_COUNT_END), RT_ERR_INPUT);
    
    if ((action_idx % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
    {
        table_index = action_idx + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
    }
    else
    {
        table_index = action_idx - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
    }
    
    osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
    /* get entry from chip */
    if ((ret = table_read(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    /* get STATISTICS */
    if ((ret = table_field_get( unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0: /*COUNT_NONE*/
            break;
        case 1: /*COUNT_PACKET*/
        case 2: /*COUNT_BYTE32*/
            if (COUNT_BYTE64 == type)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Counter 64bits can't mix with 32bits");
                return RT_ERR_FAILED;
            }
            break;       
        case 3: /*COUNT_BYTE64*/
            if ((COUNT_PACKET == type) || (COUNT_BYTE32 == type))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Counter 32bits can't mix with 64bits");
                return RT_ERR_FAILED;
            }            
            break;       
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieActionCheckMixedCounterType */

static int32 dal_esw_pie_pieActionInfoField_set(uint32 unit, pie_act_entry_t *pPie_action, uint32 action_info_field, uint32 value)
{
    int32   ret;
    uint32  field_idx;
    
    switch (action_info_field)
    {
        case 1:
            field_idx = ESW_ACTTBL_AIF1f;
            break;
        case 2:
            field_idx = ESW_ACTTBL_AIF2f;
            break;            
        case 3:
            field_idx = ESW_ACTTBL_AIF3f;
            break;
        case 4:
            field_idx = ESW_ACTTBL_AIF4f;
            break;
        case 5:
            field_idx = ESW_ACTTBL_AIF5f;
            break;
        default:
            return RT_ERR_OUT_OF_RANGE;
    }

    if ((ret = table_field_set( unit, ESW_ACTTBLt, field_idx, &value, (uint32 *) pPie_action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
            
    return RT_ERR_OK;    
}/* end of dal_esw_pie_pieActionInfoField_set */

/* Function Name:
 *      dal_esw_pie_pieRuleAction_set
 * Description:
 *      Set the PIE action configuration to specified device.
 * Input:
 *      unit       - unit id
 *      action_idx - PIE action index
 *      pAction    - PIE action configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_pieRuleAction_set(
    uint32                   unit,
    rtk_pie_id_t             action_idx,
    rtk_pie_actionTable_t    *pAction)
{
    int32           ret;
    uint32          table_index;
    uint32          value;
    uint32          action_info_field;
    uint32          opcode;
    pie_act_entry_t pie_action;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, action_idx=%d, pAction=%x", unit, action_idx, pAction);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((action_idx >= HAL_MAX_NUM_OF_PIE_ACTION(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pAction->hit_indication > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
        (pAction->statistics > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pAction->statistics != 0 && action_idx >= HAL_MAX_NUM_OF_PIE_COUNTER(unit)) ||
        (pAction->cp2cpu > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->drop > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
        (pAction->outer_tag_op > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->redirect > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->inner_tag_op > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->priority > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->dscp_remark_spid > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->police_outer_pri_remark > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
        (pAction->mirror > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));

    /*translate action index to table index*/
    table_index = action_idx;

    action_info_field = 0;
    PIE_SEM_LOCK(unit);
    
    /* set HITIND */
    switch (pAction->hit_indication)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;            
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    } 

    if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_HITINDf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STATISTICS */
    if ((ret = dal_esw_pie_pieActionCheckMixedCounterType(unit, action_idx, pAction->statistics)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }    
    switch (pAction->statistics)
    {
        case COUNT_NONE:
            value = 0;
            break;
        case COUNT_PACKET:
            value = 1;
            break;            
        case COUNT_BYTE32:
            value = 2;
            break;       
        case COUNT_BYTE64:
            value = 3;
            break;       
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Counter type value error");
            return RT_ERR_FAILED;
    }    
    if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set CP2CPU */
    switch (pAction->cp2cpu)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_CP2CPUf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DROP */
    switch (pAction->drop)
    {
        case DROP_PERMIT:
            value = 0;
            break;
        case DROP_DROP:
            value = 1;
            break;            
        case DROP_WITHDRAW_DROP:
            value = 2;
            break;       
        case DROP_RESERVED:
            value = 3;
            break;          
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Drop type value error");
            return RT_ERR_FAILED;
    }    
    if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_DROPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set OUTTAGOP */
    switch (pAction->outer_tag_op)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->outer_tag_op) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->outer_tag_op = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_OUTTAGOPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    
    /* set REDIR */
    switch (pAction->redirect)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->redirect) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->redirect = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_REDIRf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    /* set INTAGOP */
    switch (pAction->inner_tag_op)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }    
    if ((ENABLED == pAction->inner_tag_op) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->inner_tag_op = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_INTAGOPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    /* set PRI */
    switch (pAction->priority)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->priority) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->priority = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_PRIf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    /* set DSCP */
    switch (pAction->dscp_remark_spid)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->dscp_remark_spid) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->dscp_remark_spid = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_DSCPf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    /* set POLICE_OUTRMK */
    switch (pAction->police_outer_pri_remark)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->police_outer_pri_remark) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->police_outer_pri_remark = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_POLICE_OUTRMKf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    /* set MIRROR */
    switch (pAction->mirror)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            action_info_field++;
            break;         
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    if ((ENABLED == pAction->mirror) && (action_info_field > DAL_ESW_MAX_NUM_OF_ACTION_INFO_FIELD))
    {
        pAction->mirror = DISABLED;
    }
    else
    {
        if ((ret = table_field_set( unit, ESW_ACTTBLt, ESW_ACTTBL_MIRRORf, &value, (uint32 *) &pie_action)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    action_info_field = 0;
    if (ENABLED == pAction->outer_tag_op)
    {
#if 0
        /* input value range check */
        if ((pAction->outer_tag_op_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
            (pAction->outer_tag_op_field.outer_vid_ctrl > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
            (pAction->outer_tag_op_field.outer_vid_info > DAL_ESW_PIE_BIT_VALUE_MAX(12)) ||
            (pAction->outer_tag_op_field.outer_tag_op > DAL_ESW_PIE_BIT_VALUE_MAX(2)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif
        action_info_field++;
        value = 0;
        value |= (pAction->outer_tag_op_field.withdraw & 0x1) << 17;
        value |= (pAction->outer_tag_op_field.outer_vid_ctrl & 0x3) << 15;
        value |= (pAction->outer_tag_op_field.outer_vid_info & 0xfff) << 3;
        value |= (pAction->outer_tag_op_field.outer_tag_op & 0x3);   

        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    if (ENABLED == pAction->redirect)
    {
#if 0   /* input value range check */
        if ((pAction->redirect_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
            (pAction->redirect_field.opcode > DAL_ESW_PIE_BIT_VALUE_MAX(2)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif
        action_info_field++;
        value = 0;
        value |= (pAction->redirect_field.withdraw & 0x1) << 17;
        pAction->redirect_field.opcode &= 0x3;
        switch (pAction->redirect_field.opcode)
        {
            case REDIRECT_UNI_REDIRECT:
                opcode = 0;
                break;
            case REDIRECT_MULTI_REDIRECT:
                opcode = 1;
                break;
            case REDIRECT_UNI_ROUTE:
                opcode = 2;
                break;
            case REDIRECT_MULTI_ROUTE:
                opcode = 3;
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
        }
        value |= (opcode & 0x3) << 15;
    
        switch (opcode)
        {
            case 0:
                value |= (pAction->redirect_field.un.uniRedirect.cpu_tag & 0x1) << 5;
                value |= pAction->redirect_field.un.uniRedirect.dpn & 0x1f;
                break;
            case 1:
                value |= pAction->redirect_field.un.multiRedirect.fwd_idx & 0x3ff;
                break;                
            case 2:
            case 3:
                value |= (pAction->redirect_field.un.route.ttl_dec & 0x1) << 14;
                value |= pAction->redirect_field.un.route.lookup_idx & 0x3fff;
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
        }
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    if (ENABLED == pAction->inner_tag_op)
    {
#if 0   /* input value range check */
        if ((pAction->inner_tag_op_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
            (pAction->inner_tag_op_field.inner_vid_ctrl > DAL_ESW_PIE_BIT_VALUE_MAX(2)) ||
            (pAction->inner_tag_op_field.inner_vid_info > DAL_ESW_PIE_BIT_VALUE_MAX(12)) ||
            (pAction->inner_tag_op_field.inner_tag_op > DAL_ESW_PIE_BIT_VALUE_MAX(2)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif
        action_info_field++;
        value = 0;
        value |= (pAction->inner_tag_op_field.withdraw & 0x1) << 16;
        value |= (pAction->inner_tag_op_field.inner_vid_ctrl & 0x3) << 14;
        value |= (pAction->inner_tag_op_field.inner_vid_info & 0xfff) << 2;
        value |= pAction->inner_tag_op_field.inner_tag_op & 0x3;

        switch (action_info_field)
        {
            case 1:
                value = value << (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_INNER_VLAN_TAG_SIZE);
                break;
            case 2:
                value = value << (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_INNER_VLAN_TAG_SIZE);
                break;
            case 3:
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                return RT_ERR_FAILED;
        }

        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    if (ENABLED == pAction->priority)
    {
#if 0   /* input value range check */
        if ((pAction->priority_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
            (pAction->priority_field.sel_pri_dp > DAL_ESW_PIE_BIT_VALUE_MAX(3)) ||
            (pAction->priority_field.assign_pri > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
            (pAction->priority_field.acl_pri > DAL_ESW_PIE_BIT_VALUE_MAX(3)) ||
            (pAction->priority_field.assign_dp > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
            (pAction->priority_field.acl_dp > DAL_ESW_PIE_BIT_VALUE_MAX(2)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif        
        action_info_field++;
        value = 0;
        value |= (pAction->priority_field.withdraw & 0x1) << 10;
        value |= (pAction->priority_field.sel_pri_dp & 0x7) << 7;
        value |= (pAction->priority_field.assign_pri & 0x1) << 6;
        value |= (pAction->priority_field.acl_pri & 0x7) << 3;
        value |= (pAction->priority_field.assign_dp & 0x1) << 2;
        value |= pAction->priority_field.acl_dp & 0x3;

        switch (action_info_field)
        {
            case 1:
                value = value << (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                break;
            case 2:
                value = value << (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                break;
            case 3:
                value = value << (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_PRI_DP_SIZE);
                break;
            case 4:
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                return RT_ERR_FAILED;
        }

        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    /* DSCP remark and SPID use same union structure */
    if (ENABLED == pAction->dscp_remark_spid)
    {
#if 0   /* input value range check */
        if ((pAction->dscp_remark_spid_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif        
        action_info_field++;
        value = 0;
        value |= (pAction->dscp_remark_spid_field.withdraw & 0x1) << 8;
        value |= (pAction->dscp_remark_spid_field.un.dscp_remark.un.data & 0xff);
        switch (action_info_field)
        {
            case 1:
                value = value << (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                break;
            case 2:
                value = value << (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                break;
            case 3:
                value = value << (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                break;
            case 4:
                value = value << (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_DSCP_REMARK_SIZE);
                break;
            case 5:
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                return RT_ERR_FAILED;
        }
        
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    /* Policer and Outer priority remarking use same union structure */
    if (ENABLED == pAction->police_outer_pri_remark)
    {
#if 0   /* input value range check */
        if ((pAction->police_outer_pri_remark_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif  
        action_info_field++;      
        value = 0;
        value |= (pAction->police_outer_pri_remark_field.withdraw & 0x1) << 8;
        value |= pAction->police_outer_pri_remark_field.un.policer_idx & 0xff;
        switch (action_info_field)
        {
            case 1:
                value = value << (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                break;
            case 2:
                value = value << (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                break;
            case 3:
                value = value << (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                break;
            case 4:
                value = value << (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_POLICER_SIZE);
                break;
            case 5:
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                return RT_ERR_FAILED;
        }
        
        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    if (ENABLED == pAction->mirror)
    {
#if 0   /* input value range check */
        if ((pAction->mirror_field.withdraw > DAL_ESW_PIE_BIT_VALUE_MAX(1)) ||
            (pAction->mirror_field.mirror_idx > DAL_ESW_PIE_BIT_VALUE_MAX(2)))
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
            return RT_ERR_FAILED;
        }
#endif
        action_info_field++;
        value = 0;
        value |= (pAction->mirror_field.withdraw & 0x1) << 2;
        value |= pAction->mirror_field.mirror_idx & 0x3;

        switch (action_info_field)
        {
            case 1:
                value = value << (DAL_ESW_ACTION_INFO_FIELD1_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                break;
            case 2:
                value = value << (DAL_ESW_ACTION_INFO_FIELD2_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                break;
            case 3:
                value = value << (DAL_ESW_ACTION_INFO_FIELD3_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                break;
            case 4:
                value = value << (DAL_ESW_ACTION_INFO_FIELD4_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                break;
            case 5:
                value = value << (DAL_ESW_ACTION_INFO_FIELD5_LENGTH - DAL_ESW_ACTION_MIRROR_SIZE);
                break;
            default:
                PIE_SEM_UNLOCK(unit);
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
                return RT_ERR_FAILED;
        }

        if (RT_ERR_OK != dal_esw_pie_pieActionInfoField_set(unit, &pie_action, action_info_field, value))
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        }
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRuleAction_set */

/* Function Name:
 *      dal_esw_pie_pieRuleAction_del
 * Description:
 *      Delete the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - PIE action information.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Clear the content of specified PIE action to be 0.
 *      2. If the content.end_idx is smaller than content.start then clear from content.start to the maximum index of entry
 *         and then return to index 0 to continue to content.end.
 */
int32
dal_esw_pie_pieRuleAction_del(uint32 unit, rtk_pie_clearBlockContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    /* Use software clear */
    int32           ret;
    uint32          entry_idx, max_idx;
    uint32          table_index;
    pie_act_entry_t     pie_action;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_ACTION(unit);
    if ((pContent->start_idx >= max_idx) || (pContent->end_idx >= max_idx))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    if (pContent->start_idx <= pContent->end_idx)
    {
        for (entry_idx = pContent->start_idx; entry_idx <= pContent->end_idx; entry_idx++)
        {
            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }
    else
    {
        for (entry_idx = pContent->end_idx; entry_idx < max_idx; entry_idx++)
        {
            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
        for (entry_idx = 0; entry_idx <= pContent->start_idx; entry_idx++)
        {
            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            /*translate action index to table index*/
            table_index = entry_idx;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    /* Use chip clear */
    int32           ret;
    uint32          table_index;
    uint32          value;
    cbccr_entry_t   pie_action_clear;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->start_idx >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) ||
        (pContent->end_idx >= HAL_MAX_NUM_OF_PIE_ACTION(unit)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_action_clear, 0, sizeof(cbccr_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set CLR_ACTTBL */
    value = 1;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_ACTTBLf, &value, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set CLR_PIE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_CLR_PIEf, &value, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_START */
    value = pContent->start_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_STARTf, &value, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ENTRY_END */
    value = pContent->end_idx;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_ENTRY_ENDf, &value, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_CBCCRt, ESW_CBCCR_STOPALEf, &value, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_CBCCRt, table_index, (uint32 *) &pie_action_clear)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleAction_del */

/* Function Name:
 *      dal_esw_pie_pieRuleAction_move
 * Description:
 *      Move the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for move PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If the moving index of entry is greater than the maximum or smaller than the minimum index of entry,
 *      then ASIC stop action.
 */
int32
dal_esw_pie_pieRuleAction_move(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    uint64              counter64 = 0;
    int32               ret;
    uint32              i, max_idx;
    uint32              stats_type, counter32, value;
    uint32              table_from_index, table_to_index, table_index;
    pie_act_entry_t     pie_action;
    log_entry_t         pie_counter;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_ACTION(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) || 
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);

    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from + pContent->entry_length - 1;
            new_move_to = pContent->move_to + pContent->entry_length - 1;
            if (new_move_to >= max_idx)
            {
                new_move_length = max_idx - pContent->move_to;
                new_move_from = pContent->move_from + new_move_length - 1;
                new_move_to = pContent->move_to + new_move_length - 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from >= max_idx)
            {
                new_move_length = max_idx - pContent->move_from;
            }
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            new_move_direction = DIRECTION_DECREASE;
            new_move_length = pContent->entry_length;
            new_move_from = pContent->move_from;
            new_move_to = pContent->move_to;
            if (new_move_from < pContent->entry_length)
            {
                new_move_length = new_move_from + 1;
            }
        }
        else
        {
            new_move_direction = DIRECTION_INCREASE;
            new_move_length = pContent->entry_length;
            if (pContent->move_to < pContent->entry_length)
            {
                new_move_length = pContent->move_to + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
                new_move_to = 0;
            }
            else
            {
                new_move_to = pContent->move_to - new_move_length + 1;
                new_move_from = pContent->move_from - new_move_length + 1;
            }
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            table_from_index = new_move_from + i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_type, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            table_to_index = new_move_to + i;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            osal_memset(&pie_counter, 0, sizeof(log_entry_t));
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            table_from_index = new_move_from - i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_type, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            table_to_index = new_move_to - i;
            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &counter32, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            osal_memset(&pie_action, 0, sizeof(pie_act_entry_t));
            osal_memset(&pie_counter, 0, sizeof(log_entry_t));
            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_counter, 0, sizeof(log_entry_t));
                if (stats_type == 1 || stats_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_action_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) || 
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_action_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 0;/* 0 for move*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 0;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 1;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_OUT_OF_RANGE;
    }
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleAction_move */

/* Function Name:
 *      dal_esw_pie_pieRuleAction_swap
 * Description:
 *      Swap the specified PIE action.
 * Input:
 *      unit     - unit id
 *      pContent - setting for swap PIE action.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieRuleAction_swap(uint32 unit, rtk_pie_movePieContent_t *pContent)
{
#if defined(SOFTWARE_CLR_OP) /* SOFTWARE_CLR_OP */
    uint64              from_counter64 = 0, to_counter64 = 0;
    int32               ret;
    uint32              i, max_idx;
    uint32              stats_from_type, stats_to_type, from_counter32, to_counter32, value;
    uint32              table_from_index, table_to_index, table_index;
    pie_act_entry_t     pie_from_action, pie_to_action;
    log_entry_t         pie_from_counter, pie_to_counter;
    uint32              new_move_direction, new_move_length, new_move_from, new_move_to;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    max_idx = HAL_MAX_NUM_OF_PIE_ACTION(unit);
    if ((pContent->entry_length >= max_idx) ||
        (pContent->move_to >= max_idx) || 
        (pContent->move_from >= max_idx) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    new_move_direction = pContent->direction;
    new_move_length = pContent->entry_length;
    new_move_from = pContent->move_from;
    new_move_to = pContent->move_to;
    /* Calculate which entry need to be moved */
    if (DIRECTION_INCREASE == pContent->direction)
    {
        if (pContent->move_to >= pContent->move_from)
        {
            if ((pContent->move_from + new_move_length) >= pContent->move_to)
                new_move_length = pContent->move_to - pContent->move_from;
            if ((pContent->move_to + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_to;
        }
        else
        {
            if ((pContent->move_from + new_move_length - 1) >= max_idx)
                new_move_length = max_idx - pContent->move_from;
            if ((pContent->move_to + new_move_length) >= pContent->move_from)
                new_move_length = pContent->move_from - pContent->move_to;
        }
    }
    else
    {   /* DIRECTION_DECREASE == pContent->direction */
        if (pContent->move_to >= pContent->move_from)
        {
            if (new_move_from < new_move_length)
                new_move_length = new_move_from + 1;
            if ((new_move_to - new_move_length + 1) <= new_move_from)
                new_move_length = new_move_to - new_move_from;
        }
        else
        {
            if (pContent->move_to < new_move_length)
                new_move_length = pContent->move_to + 1;
            if ((new_move_from - new_move_length + 1) <= new_move_to)
                new_move_length = new_move_from - new_move_to;
        }
    }

    /* Move entry and clear from entry to 0 */
    if (DIRECTION_INCREASE == new_move_direction)
    {
        for (i = 0; i < new_move_length; i++)
        {
            table_from_index = new_move_from + i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_from_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_from_type, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            table_to_index = new_move_to + i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_to_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_to_type, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_to_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_to_counter, 0, sizeof(log_entry_t));
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = from_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((from_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_from_counter, 0, sizeof(log_entry_t));
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = to_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((to_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }
    else
    {
        for (i = 0; i < new_move_length; i++)
        {
            table_from_index = new_move_from - i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_from_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_from_type, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_from_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &from_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    from_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            table_to_index = new_move_to - i;
            if ((ret = table_read(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            /* get STATISTICS */
            stats_to_type = 0;
            if ((ret = table_field_get(unit, ESW_ACTTBLt, ESW_ACTTBL_STATISTICSf, &stats_to_type, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    if ((table_to_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    { /* get counter 63-32 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                    else
                    {/* get counter 31-0 */
                        if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &to_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                        {
                            PIE_SEM_UNLOCK(unit);
                            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                            return ret;
                        }
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    /* get counter 63-32 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 = value;
                    /* get counter 31-0 */
                    if ((ret = table_field_get(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                    to_counter64 |= ((uint64)value << 32);
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_ACTTBLt, table_to_index, (uint32 *) &pie_from_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_to_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_to_counter, 0, sizeof(log_entry_t));
                if (stats_from_type == 1 || stats_from_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &from_counter32, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_to_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_from_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_to_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = from_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((from_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_to_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }

            if ((ret = table_write(unit, ESW_ACTTBLt, table_from_index, (uint32 *) &pie_to_action)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
            if (table_from_index < HAL_MAX_NUM_OF_PIE_COUNTER(unit))
            {
                osal_memset(&pie_from_counter, 0, sizeof(log_entry_t));
                if (stats_to_type == 1 || stats_to_type == 2)
                {   /* COUNT_PACKET or COUNT_BYTE32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &to_counter32, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_from_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else if (stats_to_type == 3)
                {   /* COUNT_BYTE64 */
                    /*set lower 32bits */
                    table_index = table_from_index;
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = to_counter64 & 0xffffffff;
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
            
                    /*set higher 32bits*/
                    if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
                    {
                        table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
                    }
                    value = ((to_counter64 >> 32) & 0xffffffff);
            
                    /* set counter 63-32 */
                    if ((ret = table_field_set(unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }        
            
                    /* set entry to chip, write 32bits */
                    if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_from_counter)) != RT_ERR_OK)
                    {
                        PIE_SEM_UNLOCK(unit);
                        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                        return ret;
                    }
                }
                else
                {   /* COUNT_NONE: do nothing */
                }
            }
        }
    }

    PIE_SEM_UNLOCK(unit);
    return RT_ERR_OK;
#else
    int32               ret;
    uint32              table_index;
    uint32              value;
    move_pie_entry_t    pie_action_move;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pContent=%x", unit, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pContent->entry_length >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) ||
        (pContent->move_to >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) || 
        (pContent->move_from >= HAL_MAX_NUM_OF_PIE_ACTION(unit)) || 
        (pContent->direction > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Content value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_action_move, 0, sizeof(move_pie_entry_t));

    /*translate action index to table index*/
    table_index = 0;
    
    PIE_SEM_LOCK(unit);
    
    /* set ENTRY_LENGTH */
    value = pContent->entry_length;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ENTRY_LENGTHf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set ACTION */
    value = 1;/* 1 for swap*/
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_ACTIONf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_RULE */
    value = 0;/* move/swap PIE entry(content and mask) or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_RULEf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_ACLACTTBL */
    value = 1;/* move/swap content of ACL action table or not */
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_ACLACTTBLf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set STOPALE */
    value = 0;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_STOPALEf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set DIRECTION */
    switch (pContent->direction)
    {
        case DIRECTION_INCREASE: /* increase */
            value = 0;
            break;
        case DIRECTION_DECREASE: /* decrease */
            value = 1;
            break;       
        default: 
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_OUT_OF_RANGE;
    }
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_DIRECTIONf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_TO */
    value = pContent->move_to;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_TOf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set MOVE_FROM */
    value = pContent->move_from;
    if ((ret = table_field_set( unit, ESW_MOVE_PIEt, ESW_MOVE_PIE_MOVE_FROMf, &value, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_MOVE_PIEt, table_index, (uint32 *) &pie_action_move)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
#endif
} /* end of dal_esw_pie_pieRuleAction_swap */

/* Function Name:
 *      dal_esw_pie_pieRulePolicer_get
 * Description:
 *      Get the PIE policer configuration from specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 * Output:
 *      pPolicer    - PIE policer configuration.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_pieRulePolicer_get(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    int32           ret;
    uint32          value;    
    uint32          table_index;
    policer_entry_t pie_policer;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, policer_idx=%d", unit, policer_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((policer_idx >= HAL_MAX_NUM_OF_METERING(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pPolicer), RT_ERR_NULL_POINTER);
    
    osal_memset(&pie_policer, 0, sizeof(policer_entry_t));

    /*translate policer index to table index*/
    table_index = policer_idx;
    
    PIE_SEM_LOCK(unit);
    
    /* get entry from chip */
    if ((ret = table_read(unit, ESW_POLICERt, table_index, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* get TYPE */
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_TYPEf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pPolicer->type = POLICER_TYPE_INVALID;
            break;
        case 1:
            pPolicer->type = POLICER_TYPE_DLB;
            break;
        case 2:
            pPolicer->type = POLICER_TYPE_SRTCM;
            break;
        case 3:
            pPolicer->type = POLICER_TYPE_TRTCM;
            break;
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_FAILED;
    }

    /* get COLORAWARE */
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_COLORAWAREf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->color_aware = value;

    /* get RED_DP */
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_RED_DPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->red_dp = value;

    /* get YEL_DP */
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_YEL_DPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->yellow_dp = value;
    
    /* get CIR */
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_CIRf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->cir = value;    

    /* get PIR */    
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_PIRf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->pir = value;        
    
    /* get TC */    
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_TCf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->tc = value;           

    /* get TP */    
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_TPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->tp = value;    

    /* get CBS */    
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_CBSf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->cbs = value;  

    /* get PBS */    
    if ((ret = table_field_get( unit, ESW_POLICERt, ESW_POLICER_PBSf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pPolicer->pbs = value;  
    
    PIE_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pPolicer=%x", *pPolicer);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRulePolicer_get */

/* Function Name:
 *      dal_esw_pie_pieRulePolicer_set
 * Description:
 *      Set the PIE policer configuration to specified device.
 * Input:
 *      unit        - unit id
 *      policer_idx - PIE policer index
 *      pPolicer    - PIE policer configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
int32
dal_esw_pie_pieRulePolicer_set(
    uint32                   unit,
    rtk_pie_id_t             policer_idx,
    rtk_pie_policerEntry_t   *pPolicer)
{
    int32           ret;
    uint32          value;    
    uint32          table_index;
    policer_entry_t pie_policer;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, policer_idx=%d, pPolicer=%x", unit, policer_idx, pPolicer);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((policer_idx >= HAL_MAX_NUM_OF_METERING(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pPolicer), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pPolicer->type > DAL_ESW_PIE_BIT_VALUE_MAX(2)) || 
        (pPolicer->color_aware > DAL_ESW_PIE_BIT_VALUE_MAX(1)) || 
        (pPolicer->yellow_dp > DAL_ESW_PIE_BIT_VALUE_MAX(2)) || 
        (pPolicer->red_dp > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
        return RT_ERR_FAILED;
    }
    
    osal_memset(&pie_policer, 0, sizeof(policer_entry_t));

    /*translate policer index to table index*/
    table_index = policer_idx;

    PIE_SEM_LOCK(unit);
    
    /* set TYPE */
    switch (pPolicer->type)
    {
        case POLICER_TYPE_INVALID:
            value = 0;
            break;
        case POLICER_TYPE_DLB:
            value = 1;
            break;
        case POLICER_TYPE_SRTCM:
            value = 2;
            break;
        case POLICER_TYPE_TRTCM:
            value = 3;
            break;
        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_FAILED;
    }
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_TYPEf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    /* set COLORAWARE */
    value = pPolicer->color_aware;
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_COLORAWAREf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set RED_DP */
    value = pPolicer->red_dp;
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_RED_DPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set YEL_DP */
    value = pPolicer->yellow_dp;
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_YEL_DPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    /* set CIR */
    value = pPolicer->cir;    
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_CIRf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set PIR */
    value = pPolicer->pir;  
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_PIRf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    /* set TC */
    value = pPolicer->tc;  
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_TCf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set TP */
    value = pPolicer->tp;    
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_TPf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set CBS */
    value = pPolicer->cbs;  
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_CBSf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    /* set PBS */
    value = pPolicer->pbs;  
    if ((ret = table_field_set( unit, ESW_POLICERt, ESW_POLICER_PBSf, &value, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    /* set entry to chip */
    if ((ret = table_write(unit, ESW_POLICERt, table_index, (uint32 *) &pie_policer)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieRulePolicer_set */

/* Function Name:
 *      dal_esw_pie_pieHitIndication_get
 * Description:
 *      Get the hit indication of PIE rule from specified device.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pStatus    - hit indication status for 128 rules on a logical block.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Read and clear indication bits (128 bits/one time)
 */
int32
dal_esw_pie_pieHitIndication_get(
    uint32                       unit,
    rtk_pie_id_t                 lblock_idx,
    rtk_pie_hitIndicationEntry_t *pStatus)
{
    int32                       ret;
    uint32                      value;    
    uint32                      table_index;
    entry_hit_indi_sta_entry_t  pie_indication;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblock_idx=%d", unit, lblock_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblock_idx >= HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pStatus), RT_ERR_NULL_POINTER);
    
    osal_memset(&pie_indication, 0, sizeof(entry_hit_indi_sta_entry_t));

    /*translate policer index to table index*/
    table_index = lblock_idx;
    
    PIE_SEM_LOCK(unit);
    
    /* get entry from chip */
    if ((ret = table_read(unit, ESW_ENTRY_HIT_INDI_STAt, table_index, (uint32 *) &pie_indication)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret; 
    }

    /* get 31_0 */
    if ((ret = table_field_get( unit, ESW_ENTRY_HIT_INDI_STAt, ESW_ENTRY_HIT_INDI_STA_ELKNHIT_31_0f, &value, (uint32 *) &pie_indication)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pStatus->hit_status[0] = value & 0xff;
    pStatus->hit_status[1] = (value >> 8) & 0xff;
    pStatus->hit_status[2] = (value >> 16) & 0xff;
    pStatus->hit_status[3] = (value >> 24) & 0xff;

    /* get 63_32 */
    if ((ret = table_field_get( unit, ESW_ENTRY_HIT_INDI_STAt, ESW_ENTRY_HIT_INDI_STA_ELKNHIT_63_32f, &value, (uint32 *) &pie_indication)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pStatus->hit_status[4] = value & 0xff;
    pStatus->hit_status[5] = (value >> 8) & 0xff;
    pStatus->hit_status[6] = (value >> 16) & 0xff;
    pStatus->hit_status[7] = (value >> 24) & 0xff;

    /* get 95_64 */
    if ((ret = table_field_get( unit, ESW_ENTRY_HIT_INDI_STAt, ESW_ENTRY_HIT_INDI_STA_ELKNHIT_95_64f, &value, (uint32 *) &pie_indication)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pStatus->hit_status[8] = value & 0xff;
    pStatus->hit_status[9] = (value >> 8) & 0xff;
    pStatus->hit_status[10] = (value >> 16) & 0xff;
    pStatus->hit_status[11] = (value >> 24) & 0xff;

    /* get 127_96 */
    if ((ret = table_field_get( unit, ESW_ENTRY_HIT_INDI_STAt, ESW_ENTRY_HIT_INDI_STA_ELKNHIT_127_96f, &value, (uint32 *) &pie_indication)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pStatus->hit_status[12] = value & 0xff;
    pStatus->hit_status[13] = (value >> 8) & 0xff;
    pStatus->hit_status[14] = (value >> 16) & 0xff;
    pStatus->hit_status[15] = (value >> 24) & 0xff;
    
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pStatus=%x", *pStatus);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieHitIndication_get */

/* Function Name:
 *      dal_esw_pie_pieStat_get
 * Description:
 *      Get statistic counter of the log id from the specified device.
 * Input:
 *      unit      - unit id
 *      log_id    - log id
 * Output:
 *      pPkt_cnt  - pointer buffer of packet count
 *      pByte_cnt - pointer buffer of byte count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieStat_get(uint32 unit, rtk_pie_id_t log_id, uint32 *pPkt_cnt, uint64 *pByte_cnt)
{
    int32                   ret;
    uint32                  value;    
    uint32                  table_index;
    log_entry_t             pie_counter;
    rtk_pie_actionTable_t   action_table;
       
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, log_id=%d", unit, log_id);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((log_id >= HAL_MAX_NUM_OF_PIE_COUNTER(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pPkt_cnt), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pByte_cnt), RT_ERR_NULL_POINTER);

    /* Check this entry is 32bits or 64bits mode */
    if ((ret = dal_esw_pie_pieRuleAction_get( unit, log_id, &action_table)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    *pPkt_cnt = 0;
    *pByte_cnt = 0;
    
    if (COUNT_NONE == action_table.statistics)
    {
        return  RT_ERR_OK;
    }
    
    osal_memset(&pie_counter, 0, sizeof(log_entry_t));

    /*translate counter index to table index*/
    table_index = log_id;
    
    /* get entry from chip, read 64bits */
    PIE_SEM_LOCK(unit);
    
    if ((ret = table_read(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((COUNT_PACKET == action_table.statistics) || (COUNT_BYTE32 == action_table.statistics))
    {
        if ((log_id % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
        { /* get counter 63-32 */
            if ((ret = table_field_get( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
        else
        {/* get counter 31-0 */
            if ((ret = table_field_get( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
            {
                PIE_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
                return ret;
            }
        }
        
        if (COUNT_PACKET == action_table.statistics)
        {
            *pPkt_cnt = value;
        }
        else if (COUNT_BYTE32 == action_table.statistics)
        {
            *pByte_cnt = value;
        }
        else
        {}

    }
    else if (COUNT_BYTE64 == action_table.statistics)
    {
        /* get counter 63-32 */
        if ((ret = table_field_get( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        *pByte_cnt = value;
        /* get counter 31-0 */
        if ((ret = table_field_get( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_1f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        *pByte_cnt |= ((uint64)value << 32);
    }
    else
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
        return RT_ERR_FAILED;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pPkt_cnt=%d, pByte_cnt=%d", *pPkt_cnt, *pByte_cnt);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieStat_get */

/* Function Name:
 *      dal_esw_pie_pieStat_set
 * Description:
 *      Set statistic counter of the log id to the specified device.
 * Input:
 *      unit     - unit id
 *      log_id   - log id
 *      pkt_cnt  - packet count
 *      byte_cnt - byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_pieStat_set(uint32 unit, rtk_pie_id_t log_id, uint32 pkt_cnt, uint64 byte_cnt)
{
    int32                   ret;
    uint32                  value;    
    uint32                  table_index;
    log_entry_t             pie_counter;
    rtk_pie_actionTable_t   action_table;
               
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, log_id=%d, pkt_cnt=%ld, byte_cnt=%ld", unit, log_id, pkt_cnt, byte_cnt);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((log_id >= HAL_MAX_NUM_OF_PIE_COUNTER(unit)), RT_ERR_ENTRY_INDEX);

    /* Check this entry is 32bits or 64bits mode */
    if ((ret = dal_esw_pie_pieRuleAction_get( unit, log_id, &action_table)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if (COUNT_NONE == action_table.statistics)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
        return RT_ERR_FAILED;
    }

    osal_memset(&pie_counter, 0, sizeof(log_entry_t));

    /*translate counter index to table index*/
    table_index = log_id;
    
    PIE_SEM_LOCK(unit);
    
    if ((COUNT_PACKET == action_table.statistics) || (COUNT_BYTE32 == action_table.statistics))
    {
        if (COUNT_PACKET == action_table.statistics)
        {
            value = pkt_cnt;
        }
        else if (COUNT_BYTE32 == action_table.statistics)
        {
            value = byte_cnt & 0xffffffff;
        }
        else
        {}

        /* set counter 63-32 */
        if ((ret = table_field_set( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }        

        /* set entry to chip, write 32bits */
        if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    else if (COUNT_BYTE64 == action_table.statistics)
    {
        /*set lower 32bits */
        if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) >= (DAL_ESW_PIE_COUNTER_BLOCK / 2))
        {
            table_index = table_index - (DAL_ESW_PIE_COUNTER_BLOCK / 2);
        }
        value = byte_cnt & 0xffffffff;

        /* set counter 63-32 */
        if ((ret = table_field_set( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }        

        /* set entry to chip, write 32bits */
        if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }

        /*set higher 32bits*/
        if ((table_index % DAL_ESW_PIE_COUNTER_BLOCK) < (DAL_ESW_PIE_COUNTER_BLOCK / 2))
        {
            table_index = table_index + (DAL_ESW_PIE_COUNTER_BLOCK / 2);
        }
        value = ((byte_cnt >> 32) & 0xffffffff);

        /* set counter 63-32 */
        if ((ret = table_field_set( unit, ESW_ACL_COUNTERt, ESW_ACL_COUNTER_ACL_COUNTER_64_0f, &value, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }        

        /* set entry to chip, write 32bits */
        if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    else
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
        return RT_ERR_FAILED;
    }

    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieStat_set */

/* Function Name:
 *      dal_esw_pie_pieStat_clearAll
 * Description:
 *      Clear all statistic counter for the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 * Note:
 *      None
 */
int32
dal_esw_pie_pieStat_clearAll(uint32 unit)
{
    int32        ret;
    uint32       table_index, lblock_index, lblock_num;
    log_entry_t  pie_counter;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d", unit);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    osal_memset(&pie_counter, 0, sizeof(log_entry_t));
    lblock_num = HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit);

    PIE_SEM_LOCK(unit);

    for (lblock_index = 0; lblock_index < lblock_num; lblock_index++)
    {
        /*translate counter index to table index*/
        /*bit[10] = 1, bit[9:7] = logical block index; for logical block clear all counters*/
        table_index = 0x400; 
        table_index |= ((lblock_index&0x7) << 7);

        /* set entry to chip */
        if ((ret = table_write(unit, ESW_ACL_COUNTERt, table_index, (uint32 *) &pie_counter)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }

    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieStat_clearAll */

/* Function Name:
 *      dal_esw_pie_pieTemplateSelector_get
 * Description:
 *      Get the template index of specific physical block and lookup phase.
 * Input:
 *      unit          - unit id
 *      pblock_idx    - physical block index
 *      phase         - lookup phase
 * Output:
 *      pTemplate_idx - template index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1) pblock_idx range is 0-31
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieTemplateSelector_get(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            *pTemplate_idx)
{
    int32   ret;
    uint32  value;
    uint32  reg_idx, field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, phase=%d", unit, pblock_idx, phase);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblock_idx >= HAL_MAX_NUM_OF_PIE_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((NULL == pTemplate_idx), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL0r;
            field_idx = ESW_FCTEMPLSELf;
            break;
        case PIE_IGR_ACL:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL4r;
            field_idx = ESW_INTEMPLSELf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL8r;
            field_idx = ESW_EGTEMPLSELf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL12r;
            field_idx = ESW_EVXTEMPLSELf;
            break;                                    
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "PIE phase error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, reg_idx, pblock_idx, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    *pTemplate_idx = value;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pTemplate_idx=%d", *pTemplate_idx);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieTemplateSelector_get */


static int32 dal_esw_pie_pieCheckPhaseFieldType(uint32 unit, rtk_pie_phase_t phase, rtk_pie_id_t template_idx)
{/* check all field types in template, phase PIE_EGR_VID_TRANSLATION only supports types 0, 1, 2, 8, 9, 10 and 59. */
    uint32             field_idx; 
    rtk_pie_template_t template;

    if (PIE_EGR_VID_TRANSLATION == phase)
    {
        if (RT_ERR_OK != dal_esw_pie_pieUserTemplate_get(unit, template_idx, &template))
        {
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "User template get error");
            return RT_ERR_FAILED;
        }

        for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx++)
        {
            switch (template.field[field_idx])
            {
                case 0:
                case 1:
                case 2:
                case 8:
                case 9:
                case 10:
                case 59:
                    break;
                default:
                    RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field type error in VID translation phase");
                    return RT_ERR_FAILED;
            } /* end of switch */     
        }/* end of for ( field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx++)*/        
    } /*end of if (PIE_EGR_VID_TRANSLATION == phase)*/
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieCheckPhaseFieldType */

/* Function Name:
 *      dal_esw_pie_pieTemplateSelector_set
 * Description:
 *      Set the template index of specific physical block and lookup phase.
 * Input:
 *      unit         - unit id
 *      pblock_idx   - physical block index
 *      phase        - lookup phase
 *      template_idx - template index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      1) pblock_idx range is 0-31, template_idx range is 0-15
 *      2) RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_pieTemplateSelector_set(
    uint32                  unit,
    rtk_pie_id_t            pblock_idx,
    rtk_pie_phase_t         phase,
    rtk_pie_id_t            template_idx)
{
    int32   ret;
    uint32  value;
    uint32  reg_idx, field_idx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, phase=%d, template_idx=%d", unit, pblock_idx, phase, template_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblock_idx >= HAL_MAX_NUM_OF_PIE_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((template_idx >= HAL_MAX_NUM_OF_PIE_TEMPLATE(unit)), RT_ERR_INPUT);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL0r;
            field_idx = ESW_FCTEMPLSELf;
            break;
        case PIE_IGR_ACL:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL4r;
            field_idx = ESW_INTEMPLSELf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL8r;
            field_idx = ESW_EGTEMPLSELf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            if (0 == template_idx)
            {/* Predefined template can't bind on EGR VID TRANSLATION */
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }                
            if (RT_ERR_OK != dal_esw_pie_pieCheckPhaseFieldType(unit, phase, template_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            reg_idx = ESW_TEMPLATE_SELECTOR_CONTROL12r;
            field_idx = ESW_EVXTEMPLSELf;
            break;                                    
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Phase type error");
            return RT_ERR_FAILED;
    }

    value = template_idx;

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_array_field_write(unit, reg_idx, pblock_idx, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieTemplateSelector_set */

/* Function Name:
 *      dal_esw_pie_pieUserTemplate_get
 * Description:
 *      Get the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 * Output:
 *      pTemplate    - template content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieUserTemplate_get(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    int32   ret;
    uint32  field_idx;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, template_idx=%d", unit, template_idx);

    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((template_idx >= HAL_MAX_NUM_OF_PIE_TEMPLATE(unit)), RT_ERR_ENTRY_INDEX);
    /*index 0 is for predefined template*/
    RT_PARAM_CHK((0 == template_idx), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((NULL == pTemplate), RT_ERR_NULL_POINTER);

    PIE_SEM_LOCK(unit);
    
     /* get value from CHIP */
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx++)
    {
        if ((ret = reg_array_field_read(unit, ESW_USER_DEFINED_TEMPLATE_CONTROLr, template_idx, field_idx, ESW_FIELDf, &value)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        pTemplate->field[field_idx] = value;
    }
 
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pTemplate=%x", *pTemplate);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieUserTemplate_get */

/*
 * Some field types only can locate in specified field.
 */
static int32 dal_esw_pie_pieCheckFieldLocation(rtk_pie_id_t field_idx, rtk_pie_templateFiledType_t field_type)
{
    /* Check arguments */
    RT_PARAM_CHK((field_idx >= RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((field_type >= PIE_FIELD_TYPE_END), RT_ERR_ENTRY_INDEX);
    
    switch (field_type)
    {
        case FMT:
            if (8 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case DMAC0:
        case SMAC0:            
            if ((3 != field_idx) && (7 != field_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case DMAC1_RRCPINFO:
        case SMAC1:
            if ((0 != field_idx) && (4 != field_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case DMAC2:
        case SMAC2:            
            if ((1 != field_idx) && (5 != field_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP2:
        case DIP2:
            if (4 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP3:
        case DIP3:
            if (5 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP4:
        case DIP4:
            if (2 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP5:
        case DIP5:
            if (3 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP6:
        case DIP6:
            if (0 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case SIP7:
        case DIP7:
            if (1 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case IP_RANG1:
            if (2 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case IP_RANG2:
            if (3 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case IP_RANG3:
            if (0 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case IP_RANG4:
            if (1 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case PATTERN_MATCH0:
        case PAYLOAD0:
            if ((2 != field_idx) && (6 != field_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case PATTERN_MATCH1:
        case PAYLOAD1:
            if ((3 != field_idx) && (7 != field_idx))
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR1_0:
            if (2 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR1_1:
            if (3 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR1_2:
            if (0 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR1_3:
            if (1 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR2_0:
            if (6 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR2_1:
            if (7 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR2_2:
            if (4 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        case FIELD_SELECTOR2_3:
            if (5 != field_idx)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
                return RT_ERR_FAILED;
            }
            break;

        default:
            break;
    }
 
    return RT_ERR_OK;
}/* end of dal_esw_pie_pieCheckFieldLocation */

/* Function Name:
 *      dal_esw_pie_pieUserTemplate_set
 * Description:
 *      Set the template content of specific template index.
 * Input:
 *      unit         - unit id
 *      template_idx - template index
 *      pTemplate    - template content
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieUserTemplate_set(uint32 unit, rtk_pie_id_t template_idx, rtk_pie_template_t *pTemplate)
{
    int32   ret;
    uint32  field_idx;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, template_idx=%d, pTemplate=%x", unit, template_idx, pTemplate);

    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((template_idx >= HAL_MAX_NUM_OF_PIE_TEMPLATE(unit)), RT_ERR_ENTRY_INDEX);
    /*index 0 is for predefined template*/
    RT_PARAM_CHK((0 == template_idx), RT_ERR_ENTRY_INDEX);  
    RT_PARAM_CHK((NULL == pTemplate), RT_ERR_NULL_POINTER);

    /* set value to CHIP */
    for (field_idx = 0; field_idx < RTK_MAX_NUM_OF_PIE_TEMPLATE_FIELD; field_idx++)
    {
        if ((ret = dal_esw_pie_pieCheckFieldLocation(field_idx, pTemplate->field[field_idx])) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        value = pTemplate->field[field_idx];

        PIE_SEM_LOCK(unit);
        if ((ret = reg_array_field_write(unit, ESW_USER_DEFINED_TEMPLATE_CONTROLr, template_idx, field_idx, ESW_FIELDf, &value)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        PIE_SEM_UNLOCK(unit);
    }
 
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieUserTemplate_set */

/* Function Name:
 *      dal_esw_pie_pieL34ChecksumErr_get
 * Description:
 *      Get the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit       - unit id
 * Output:
 *      pOperation - operation for L3 and L4 packets checksum error
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieL34ChecksumErr_get(uint32 unit, rtk_pie_l34ChecksumErrOper_t *pOperation)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d", unit);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pOperation), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, ESW_LOOKUP_ENGINE_TEMPLATE_GENERATOR_CONTROLr, ESW_TGL34ERROPf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pOperation = CHECKSUM_ERR_DOWNGRADE;
            break;
        case 1:
            *pOperation = CHECKSUM_ERR_PARSE_ANYWAY;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pOperation=%x", *pOperation);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieL34ChecksumErr_get */

/* Function Name:
 *      dal_esw_pie_pieL34ChecksumErr_set
 * Description:
 *      Set the operation for L3 and L4 packets checksum error.
 * Input:
 *      unit      - unit id
 *      operation - operation for L3 and L4 packets checksum error
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_pieL34ChecksumErr_set(uint32 unit, rtk_pie_l34ChecksumErrOper_t operation)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, operation=%d", unit, operation);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((operation >= PIE_CHECKSUM_ERR_OPER_END), RT_ERR_INPUT);

    switch (operation)
    {
        case CHECKSUM_ERR_DOWNGRADE:
            value = 0;
            break;
        case CHECKSUM_ERR_PARSE_ANYWAY:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Checksum error value error");
            return RT_ERR_FAILED;
    }
    
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP */
    if ((ret = reg_field_write(unit, ESW_LOOKUP_ENGINE_TEMPLATE_GENERATOR_CONTROLr, ESW_TGL34ERROPf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieL34ChecksumErr_set */

/* Function Name:
 *      dal_esw_pie_pieUserTemplatePayloadOffset_get
 * Description:
 *      Get the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 * Output:
 *      pOffset    - payload offset
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieUserTemplatePayloadOffset_get(
    uint32          unit,
    rtk_pie_id_t    pblock_idx,
    rtk_pie_id_t    offset_idx,
    uint32          *pOffset)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, offset_idx=%d", unit, pblock_idx, offset_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblock_idx >= HAL_MAX_NUM_OF_PIE_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((offset_idx >= HAL_MAX_NUM_OF_PIE_PAYLOAD(unit)), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((NULL == pOffset), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, (ESW_USER_DEFINED_TEMPLATE_PAYLOAD_OFFSET_CONTROL0r + (pblock_idx / 2)), templatePayloadOffset_fieldIdx[pblock_idx][offset_idx], &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    *pOffset = value;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pOffset=%x", *pOffset);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieUserTemplatePayloadOffset_get */

/* Function Name:
 *      dal_esw_pie_pieUserTemplatePayloadOffset_set
 * Description:
 *      Set the payload offset for template field that bound in secific physical block.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      offset_idx - offset index
 *      offset     - payload offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
dal_esw_pie_pieUserTemplatePayloadOffset_set(
    uint32          unit,
    rtk_pie_id_t    pblock_idx,
    rtk_pie_id_t    offset_idx,
    uint32          offset)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, offset_idx=%d, offset=0x%x", unit, pblock_idx, offset_idx, offset);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblock_idx >= HAL_MAX_NUM_OF_PIE_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((offset_idx >= HAL_MAX_NUM_OF_PIE_PAYLOAD(unit)), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((offset > RTK_MAX_NUM_OF_PAYLOAD_OFFSET), RT_ERR_OUT_OF_RANGE);

    value = offset;
            
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP */
    if ((ret = reg_field_write(unit, (ESW_USER_DEFINED_TEMPLATE_PAYLOAD_OFFSET_CONTROL0r + (pblock_idx / 2)), templatePayloadOffset_fieldIdx[pblock_idx][offset_idx], &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieUserTemplatePayloadOffset_set */    

/* Function Name:
 *      dal_esw_pie_pieResultReverse_get
 * Description:
 *      Get the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      entry_idx  - entry index
 * Output:
 *      pOperation - result reverse
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieResultReverse_get(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  *pOperation)
{
    int32   ret;
    uint32  value = 0;
    uint32  pblock_idx = 0;
    uint32  pblock_entry_idx = 0;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, entry_idx=%d", unit, pblock_idx, entry_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((NULL == pOperation), RT_ERR_NULL_POINTER);

    pblock_idx = entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    pblock_entry_idx = entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PIE_RESULT_REVERSE_CONTROLr, ((pblock_idx * 2) + (pblock_entry_idx / 32)), REG_ARRAY_INDEX_NONE, ESW_PIERRf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    PIE_SEM_UNLOCK(unit);
    
    if (0 == ((value >> (pblock_entry_idx % 32)) & 0x1))
    {
        *pOperation = RESULT_KEEP;
    }
    else
    {
        *pOperation = RESULT_REVERSE;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "*pOperation=%d", *pOperation);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieResultReverse_get */

/* Function Name:
 *      dal_esw_pie_pieResultReverse_set
 * Description:
 *      Set the operation of reverse for lookup result.
 * Input:
 *      unit       - unit id
 *      pblock_idx - physical block index
 *      entry_idx  - entry index
 *      operation  - result reverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_pieResultReverse_set(
    uint32                       unit,
    rtk_pie_id_t                 entry_idx,
    rtk_pie_resultReverseOper_t  operation)
{
    int32   ret;
    uint32  value = 0;
    uint32  reverse_bit = 0;
    uint32  pblock_idx = 0;
    uint32  pblock_entry_idx = 0;    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblock_idx=%d, entry_idx=%d, operation=%d", unit, pblock_idx, entry_idx, operation);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_FILTER_ID(unit)), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((operation >= PIE_RESULT_REVERSE_END), RT_ERR_INPUT);

    pblock_idx = entry_idx / HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    pblock_entry_idx = entry_idx % HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit);
    
    if (RESULT_KEEP == operation)
    {
        reverse_bit = 0;
    }
    else if (RESULT_REVERSE == operation)
    {
        reverse_bit = 1;
    }
    else
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Result reverse value error");
        return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PIE_RESULT_REVERSE_CONTROLr, ((pblock_idx * 2) + (pblock_entry_idx / 32)), REG_ARRAY_INDEX_NONE, ESW_PIERRf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    value &= ~(1 << (pblock_entry_idx % 32));
    value |= (reverse_bit << (pblock_entry_idx % 32));
    if ((ret = reg_array_field_write(unit, ESW_PIE_RESULT_REVERSE_CONTROLr, ((pblock_idx * 2) + (pblock_entry_idx / 32)), REG_ARRAY_INDEX_NONE, ESW_PIERRf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieResultReverse_set */

/* Function Name:
 *      dal_esw_pie_pieResultAggregator_get
 * Description:
 *      Get the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 * Output:
 *      pType           - type of result aggregator
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
int32
dal_esw_pie_pieResultAggregator_get(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   *pType)
{
    int32   ret;
    uint32  value = 0;
    uint32  data = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblockRange_idx=%d, entry_idx=%d", unit, pblockRange_idx, entry_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblockRange_idx >= PIE_AGGREGATOR_RANGE_END), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pType), RT_ERR_NULL_POINTER);

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PIE_RESULT_AGGREGATOR_GLOBAL_CONTROLr, ((pblockRange_idx * 8) + (entry_idx / 8)), REG_ARRAY_INDEX_NONE, ESW_PIERAf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    data = (value >> ((entry_idx % 8) * 4)) & 0xf;
    /* chip's value translate */
    switch (data)
    {
        case 0x4: /*0b0100*/
            *pType = A01M0;
            break;
        case 0x5: /*0b0101*/
            *pType = A01M1;
            break;
        case 0x6: /*0b0110*/
            *pType = A23M2;
            break;
        case 0x7: /*0b0111*/
            *pType = A23M3;
            break;
        case 0x8: /*0b1000*/
            *pType = A01_23M0_2;
            break;
        case 0x9: /*0b1001*/
            *pType = A01_23M1_2;
            break;
        case 0xa: /*0b1010*/
            *pType = A01_23M0_3;
            break;
        case 0xb: /*0b1011*/
            *pType = A01_23M1_3;
            break;
        case 0xc: /*0b1100*/
            *pType = A0123M0;
            break;
        case 0xd: /*0b1101*/
            *pType = A0123M1;
            break;  
        case 0xe: /*0b1110*/
            *pType = A0123M2;
            break;
        case 0xf: /*0b1111*/
            *pType = A0123M3;
            break;                                                                         
        default:
            *pType = NONE;
            break;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pType=%x", *pType);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieResultAggregator_get */

/* Function Name:
 *      dal_esw_pie_pieResultAggregator_set
 * Description:
 *      Set the type of lookup result aggregation.
 * Input:
 *      unit            - unit id
 *      pblockRange_idx - index of physical block range
 *      entry_idx       - physical block entry index
 *      type            - type of result aggregator
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      The range index: 0 for physical block 0~3, 1 for physical block 4~7, ..., 7 for physical block 28~31.
 */
int32
dal_esw_pie_pieResultAggregator_set(
    uint32                           unit,
    rtk_pie_resultAggregatorRange_t  pblockRange_idx,
    rtk_pie_id_t                     entry_idx,
    rtk_pie_resultAggregatorType_t   type)
{
    int32   ret;
    uint32  value = 0;
    uint32  data = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pblockRange_idx=%d, entry_idx=%d, type=%d", unit, pblockRange_idx, entry_idx, type);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((pblockRange_idx >= PIE_AGGREGATOR_RANGE_END), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((entry_idx >= HAL_MAX_NUM_OF_PIE_BLOCKSIZE(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((type >= PIE_RESULT_AGGREGATOR_TYPE_END), RT_ERR_INPUT);

    /* chip's value translate */
    switch (type)
    {
        case NONE:
            data = 0;
            break;        
        case A01M0:
            data = 0x4; /*0b0100*/
            break;
        case A01M1:
            data = 0x5; /*0b0101*/
            break;
        case A23M2:
            data = 0x6; /*0b0110*/
            break;
        case A23M3:
            data = 0x7; /*0b0111*/
            break;
        case A01_23M0_2:
            data = 0x8; /*0b1000*/
            break;
        case A01_23M1_2:
            data = 0x9; /*0b1001*/
            break;
        case A01_23M0_3:
            data = 0xa; /*0b1010*/
            break;
        case A01_23M1_3:
            data = 0xb; /*0b1011*/
            break;
        case A0123M0:
            data = 0xc; /*0b1100*/
            break;
        case A0123M1:
            data = 0xd; /*0b1101*/
            break;  
        case A0123M2:
            data = 0xe; /*0b1110*/
            break;
        case A0123M3:
            data = 0xf; /*0b1111*/
            break;                                                                         
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Result aggregator value error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PIE_RESULT_AGGREGATOR_GLOBAL_CONTROLr, ((pblockRange_idx * 8) + (entry_idx / 8)), REG_ARRAY_INDEX_NONE, ESW_PIERAf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    value &= ~(0xf << ((entry_idx % 8) * 4));
    value |= (data << ((entry_idx % 8) * 4));
    /* set value to CHIP */
    if ((ret = reg_array_field_write(unit, ESW_PIE_RESULT_AGGREGATOR_GLOBAL_CONTROLr, ((pblockRange_idx * 8) + (entry_idx / 8)), REG_ARRAY_INDEX_NONE, ESW_PIERAf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieResultAggregator_set */

/* Function Name:
 *      dal_esw_pie_pieBlockPriority_get
 * Description:
 *      Get the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pPriority  - block priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieBlockPriority_get(uint32 unit, rtk_pie_id_t lblock_idx, uint32 *pPriority)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblock_idx=%d", unit, lblock_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblock_idx >= HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pPriority), RT_ERR_NULL_POINTER);

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PIE_BLOCK_PRIORITY_CONTROLr, lblock_idx, REG_ARRAY_INDEX_NONE, ESW_PIEBPRIf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    *pPriority = value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pPriority=%x", *pPriority);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieBlockPriority_get */

/* Function Name:
 *      dal_esw_pie_pieBlockPriority_set
 * Description:
 *      Set the priority of PIE block.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      priority   - block priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter 
 * Note:
 *      None
 */
int32
dal_esw_pie_pieBlockPriority_set(uint32 unit, rtk_pie_id_t lblock_idx, uint32 priority)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblock_idx=%d, priority=%d", unit, lblock_idx, priority);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblock_idx >= HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((priority > RTK_PIE_BLOCK_PRIORITY_MAX), RT_ERR_INPUT);

    value = priority;
    
    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_array_field_write(unit, ESW_PIE_BLOCK_PRIORITY_CONTROLr, lblock_idx, REG_ARRAY_INDEX_NONE, ESW_PIEBPRIf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieBlockPriority_set */

/* Function Name:
 *      dal_esw_pie_pieGroupCtrl_get
 * Description:
 *      Get the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 * Output:
 *      pOperation      - group operation
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
int32
dal_esw_pie_pieGroupCtrl_get(
    uint32                      unit, 
    rtk_pie_groupCtrlRange_t    lblockRange_idx, 
    rtk_pie_groupCtrl_t         *pOperation)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblockRange_idx=%d", unit, lblockRange_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblockRange_idx >= PIE_GROUP_RANGE_END), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((NULL == pOperation), RT_ERR_NULL_POINTER);

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_field_read(unit, ESW_PACKET_INSPECTION_ENGINE_GROUP_CONTROLr, (lbolckGroupControl_fieldIdx[lblockRange_idx]), &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0x0: /*0b00*/
            *pOperation = GROUP_NONE;
            break;
        case 0x1: /*0b01*/
            *pOperation = GROUP_01;
            break;
        case 0x2: /*0b10*/
            *pOperation = GROUP_012;
            break;
        case 0x3: /*0b11*/
            *pOperation = GROUP_0123;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Group value error");
            return RT_ERR_FAILED;
    }
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pOperation=%x", *pOperation);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieGroupCtrl_get */

/* Function Name:
 *      dal_esw_pie_pieGroupCtrl_set
 * Description:
 *      Set the group operation of logical block.
 * Input:
 *      unit            - unit id
 *      lblockRange_idx - index of logical block range
 *      operation       - group operation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      The range index: 0 for logical block index 0~3, 1 for logical block index 4~7, ..., 3 for logical block index 12~15
 */
int32
dal_esw_pie_pieGroupCtrl_set(
    uint32                      unit, 
    rtk_pie_groupCtrlRange_t    lblockRange_idx, 
    rtk_pie_groupCtrl_t         operation)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblockRange_idx=%d, operation=%d", unit, lblockRange_idx, operation);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblockRange_idx >= PIE_GROUP_RANGE_END), RT_ERR_ENTRY_INDEX);    
    RT_PARAM_CHK((operation >= PIE_GROUP_END), RT_ERR_INPUT);

    /* chip's value translate */
    switch (operation)
    {
        case GROUP_NONE:
            value = 0x0; /*0b00*/
            break;
        case GROUP_01:
            value = 0x1; /*0b01*/
            break;
        case GROUP_012:
            value = 0x2; /*0b10*/
            break;
        case GROUP_0123:
            value = 0x3; /*0b11*/
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Group value error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_field_write(unit, ESW_PACKET_INSPECTION_ENGINE_GROUP_CONTROLr, (lbolckGroupControl_fieldIdx[lblockRange_idx]), &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieGroupCtrl_set */

/* Function Name:
 *      dal_esw_pie_pieEgrAclLookupCtrl_get
 * Description:
 *      Get the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      RTL8328S is not supported the function.
 */
int32
dal_esw_pie_pieEgrAclLookupCtrl_get(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    int32   ret;
    uint32  flood_miss_action = 0;
    uint32  flood_miss_enable = 0;
    uint32  mcast_miss_action = 0;
    uint32  mcast_miss_enable = 0;
    uint32  other_drop_enable = 0;        
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d", unit);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pControl), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)), RT_ERR_CHIP_NOT_SUPPORTED);

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_field_read(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_FLDDEFACTf, &flood_miss_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_read(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_FLDEGACLENf, &flood_miss_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_read(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_MCSTDEFACTf, &mcast_miss_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_read(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_MCSTEGACLENf, &mcast_miss_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_read(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_DROPEGACLENf, &other_drop_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (flood_miss_action)
    {
        case 0:
            pControl->flood_lookupMiss_action = LOOKUP_MISS_PERMIT;
            break;
        case 1:
            pControl->flood_lookupMiss_action = LOOKUP_MISS_DROP;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss action value error");
            return RT_ERR_FAILED;
    }

    switch (flood_miss_enable)
    {
        case 0:
            pControl->flood_egrAcl_enable = DISABLED;
            break;
        case 1:
            pControl->flood_egrAcl_enable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }

    switch (mcast_miss_action)
    {
        case 0:
            pControl->mcast_lookupMiss_action = LOOKUP_MISS_PERMIT;
            break;
        case 1:
            pControl->mcast_lookupMiss_action = LOOKUP_MISS_DROP;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss action value error");
            return RT_ERR_FAILED;
    }

    switch (mcast_miss_enable)
    {
        case 0:
            pControl->mcast_egrAcl_enable = DISABLED;
            break;
        case 1:
            pControl->mcast_egrAcl_enable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }

    switch (other_drop_enable)
    {
        case 0:
            pControl->other_drop_enable = DISABLED;
            break;
        case 1:
            pControl->other_drop_enable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pControl=%x", *pControl);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieEgrAclLookupCtrl_get */

/* Function Name:
 *      dal_esw_pie_pieEgrAclLookupCtrl_set
 * Description:
 *      Set the control configuration of egress ACL lookup.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      RTL8328S is not supported the function.
 */
int32
dal_esw_pie_pieEgrAclLookupCtrl_set(uint32 unit, rtk_pie_egrAclLookupCtrl_t *pControl)
{
    int32   ret;
    uint32  flood_miss_action = 0;
    uint32  flood_miss_enable = 0;
    uint32  mcast_miss_action = 0;
    uint32  mcast_miss_enable = 0;
    uint32  other_drop_enable = 0;        
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pControl=%x", unit, pControl);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pControl), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)), RT_ERR_CHIP_NOT_SUPPORTED);

    /* chip's value translate */
    switch (pControl->flood_lookupMiss_action)
    {
        case LOOKUP_MISS_PERMIT:
            flood_miss_action = 0;
            break;
        case LOOKUP_MISS_DROP:
            flood_miss_action = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss action value error");
            return RT_ERR_FAILED;
    }

    switch (pControl->flood_egrAcl_enable)
    {
        case DISABLED:
            flood_miss_enable = 0;
            break;
        case ENABLED:
            flood_miss_enable = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }

    switch (pControl->mcast_lookupMiss_action)
    {
        case LOOKUP_MISS_PERMIT:
            mcast_miss_action = 0;
            break;
        case LOOKUP_MISS_DROP:
            mcast_miss_action = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss action value error");
            return RT_ERR_FAILED;
    }

    switch (pControl->mcast_egrAcl_enable)
    {
        case DISABLED:
            mcast_miss_enable = 0;
            break;
        case ENABLED:
            mcast_miss_enable = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }

    switch (pControl->other_drop_enable)
    {
        case DISABLED:
            other_drop_enable = 0;
            break;
        case ENABLED:
            other_drop_enable = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss enable value error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_field_write(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_FLDDEFACTf, &flood_miss_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_write(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_FLDEGACLENf, &flood_miss_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_write(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_MCSTDEFACTf, &mcast_miss_action)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_write(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_MCSTEGACLENf, &mcast_miss_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_field_write(unit, ESW_GLOBAL_EGRESS_ACL_LOOKUP_CONTROLr, ESW_DROPEGACLENf, &other_drop_enable)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieEgrAclLookupCtrl_set */

/* Function Name:
 *      dal_esw_pie_piePortLookupPhaseEnable_get
 * Description:
 *      Get the enable status of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_piePortLookupPhaseEnable_get(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     *pEnable)
{
    int32   ret;
    uint32  value = 0;
    uint32  reg_idx;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, phase=%d", unit, port, phase);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION:
            reg_idx = ESW_PORT_FLOW_CLASSIFICATION_LOOKUP_CONTROLr;
            field_idx = ESW_FCLUENf;
            break;
        case PIE_IGR_ACL:
            reg_idx = ESW_PORT_INGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_INACLLUENf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_PORT_EGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_EGACLLUENf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            reg_idx = ESW_PORT_EGRESS_VID_TRANSLATE_LOOKUP_CONTROLr;
            field_idx = ESW_VIDTRAN_ENf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "PIE phase error");
            return RT_ERR_FAILED;
    }
       
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
  
    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pEnable=%d", *pEnable);

    return RT_ERR_OK;
} /* end of dal_esw_pie_piePortLookupPhaseEnable_get */

/* Function Name:
 *      dal_esw_pie_piePortLookupPhaseEnable_set
 * Description:
 *      Set the enable status of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_piePortLookupPhaseEnable_set(
    uint32           unit,
    rtk_port_t       port,
    rtk_pie_phase_t  phase,
    rtk_enable_t     enable)
{
    int32   ret;
    uint32  value = 0;
    uint32  reg_idx;
    uint32  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, phase=%d, enable=%d", unit, port, phase, enable);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    /* chip's value translate */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), " ");
            return RT_ERR_FAILED;
    }

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION:
            reg_idx = ESW_PORT_FLOW_CLASSIFICATION_LOOKUP_CONTROLr;
            field_idx = ESW_FCLUENf;
            break;
        case PIE_IGR_ACL:
            reg_idx = ESW_PORT_INGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_INACLLUENf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_PORT_EGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_EGACLLUENf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            reg_idx = ESW_PORT_EGRESS_VID_TRANSLATE_LOOKUP_CONTROLr;
            field_idx = ESW_VIDTRAN_ENf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "PIE phase error");
            return RT_ERR_FAILED;
    }
       
    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_piePortLookupPhaseEnable_set */

/* Function Name:
 *      dal_esw_pie_piePortLookupPhaseMiss_get
 * Description:
 *      Get the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      phase   - lookup_phase
 * Output:
 *      pAction - lookup miss action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_piePortLookupPhaseMiss_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   *pAction)
{
    int32   ret;
    uint32  value = 0;
    uint32  reg_idx, field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, phase=%d", unit, port, phase);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION: /* refer to IGR_ACL setting */
        case PIE_IGR_ACL:
            reg_idx = ESW_PORT_INGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_DEFACTf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_PORT_EGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_DEFACTf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            reg_idx = ESW_PORT_EGRESS_VID_TRANSLATE_LOOKUP_CONTROLr;
            field_idx = ESW_VIDTRANDEFACTf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "PIE phase error");
            return RT_ERR_FAILED;
    }
       
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pAction = LOOKUP_MISS_PERMIT;
            break;
        case 1:
            *pAction = LOOKUP_MISS_DROP;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup action value error");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pAction=%d", *pAction);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_piePortLookupPhaseMiss_get */

/* Function Name:
 *      dal_esw_pie_piePortLookupPhaseMiss_set
 * Description:
 *      Set the lookup miss action of specific lookup phase on a port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      phase  - lookup_phase
 *      action - lookup miss action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PIE_PHASE    - invalid PIE phase 
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      RTL8328S is not supported PIE_EGR_ACL and PIE_EGR_VID_TRANSLATION phase.
 */
int32
dal_esw_pie_piePortLookupPhaseMiss_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_phase_t              phase,
    rtk_pie_lookupMissAction_t   action)
{
    int32   ret;
    uint32  value = 0;
    uint32  reg_idx, field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, phase=%d, action=%d", unit, port, phase, action);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((phase >= PIE_PHASE_END), RT_ERR_PIE_PHASE);
    RT_PARAM_CHK((action >= PIE_LOOKUP_MISS_END), RT_ERR_INPUT);
    RT_PARAM_CHK(((HAL_IS_MAC_8328S(unit) || HAL_IS_MAC_8328L(unit)) && (phase >= PIE_EGR_ACL)), RT_ERR_PIE_PHASE_NOT_SUPPORTED);

    /* chip's value translate */
    switch (action)
    {
        case LOOKUP_MISS_PERMIT:
            value = 0;
            break;
        case LOOKUP_MISS_DROP:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Lookup miss action value error");
            return RT_ERR_FAILED;
    }

    switch (phase)
    {
        case PIE_FLOW_CLASSIFICATION: /* refer to IGR_ACL setting */
        case PIE_IGR_ACL:
            reg_idx = ESW_PORT_INGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_DEFACTf;
            break;
        case PIE_EGR_ACL:
            reg_idx = ESW_PORT_EGRESS_ACL_LOOKUP_CONTROLr;
            field_idx = ESW_DEFACTf;
            break;
        case PIE_EGR_VID_TRANSLATION:
            reg_idx = ESW_PORT_EGRESS_VID_TRANSLATE_LOOKUP_CONTROLr;
            field_idx = ESW_VIDTRANDEFACTf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "PIE phase error");
            return RT_ERR_FAILED;
    }
       
    PIE_SEM_LOCK(unit);

    /* set value tom CHIP */
    if ((ret = reg_array_field_write(unit, reg_idx, port, REG_ARRAY_INDEX_NONE, field_idx, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_piePortLookupPhaseMiss_set */

/* Function Name:
 *      dal_esw_pie_pieCounterIndicationMode_get
 * Description:
 *      Get the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 * Output:
 *      pMode      - counter and hit indication mode.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_pieCounterIndicationMode_get(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  *pMode)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblock_idx=%d", unit, lblock_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblock_idx >= HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_ACL_BLOCK_COUNTER_AND_INDICATION_MODE_CONTROLr, lblock_idx, REG_ARRAY_INDEX_NONE, ESW_ACLRMMODf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    PIE_SEM_UNLOCK(unit);

    switch (value)
    {
        case 0:
            *pMode = ACTION_EXECUTION;
            break;
        case 1:
            *pMode = RULE_MATCH;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Indication mode value error");
            return RT_ERR_FAILED;
    }        
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pMode=%d", *pMode);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_pieCounterIndicationMode_get */

/* Function Name:
 *      dal_esw_pie_pie_counterIndicationMode_set
 * Description:
 *      Set the mode of ACL counter and hit indication.
 * Input:
 *      unit       - unit id
 *      lblock_idx - logical block index
 *      mode       - counter and hit indication mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_pieCounterIndicationMode_set(
    uint32                           unit,
    rtk_pie_id_t                     lblock_idx,
    rtk_pie_counterIndicationMode_t  mode)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, lblock_idx=%d, mode=%d", unit, lblock_idx, mode);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((lblock_idx >= HAL_MAX_NUM_OF_PIE_LOGICAL_BLOCK(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((mode >= PIE_INDICATION_MODE_END), RT_ERR_INPUT);

    switch (mode)
    {
        case ACTION_EXECUTION:
            value = 0;
            break;
        case RULE_MATCH:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Indication mode value error");
            return RT_ERR_FAILED;
    }                    

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    if ((ret = reg_array_field_write(unit, ESW_ACL_BLOCK_COUNTER_AND_INDICATION_MODE_CONTROLr, lblock_idx, REG_ARRAY_INDEX_NONE, ESW_ACLRMMODf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_pieCounterIndicationMode_set */    

/* Function Name:
 *      dal_esw_pie_piePolicerCtrl_get
 * Description:
 *      Get the control configuration of policer.
 * Input:
 *      unit     - unit id
 * Output:
 *      pControl - control configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_piePolicerCtrl_get(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d", unit);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pControl), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, ESW_POLICER_TABLE_CONTROLr, ESW_INCLPREIFGf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    pControl->preamble_ifg = value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pControl=%d", *pControl);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_piePolicerCtrl_get */

/* Function Name:
 *      dal_esw_pie_piePolicerCtrl_set
 * Description:
 *      Set the control configuration of policer.
 * Input:
 *      unit     - unit id
 *      pControl - control configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_piePolicerCtrl_set(uint32 unit, rtk_pie_policerCtrl_t *pControl)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, pControl=%x", unit, pControl);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((NULL == pControl), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pControl->preamble_ifg > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
        return RT_ERR_FAILED;
    }

    value = pControl->preamble_ifg;
        
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP */
    if ((ret = reg_field_write(unit, ESW_POLICER_TABLE_CONTROLr, ESW_INCLPREIFGf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_piePolicerCtrl_set */

/* Function Name:
 *      dal_esw_pie_rangeCheckL4Port_get
 * Description:
 *      Get the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of L4 port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckL4Port_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d", unit, index);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_L4PORT(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_PORT_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->upper_bound = value;
    
    if ((ret = reg_field_read(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_PORT_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->lower_bound = value;
    
    if ((ret = reg_field_read(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_DESTPORTf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pData->dest_port = L4PORT_DIRECTION_SRC;
            break;

        case 1:
            pData->dest_port = L4PORT_DIRECTION_DST;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L4 port type value error");
            return RT_ERR_FAILED;
    }    
    
    if ((ret = reg_field_read(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pData->l4port_type = L4PORT_TYPE_INVALID;
            break;

        case 1:
            pData->l4port_type = L4PORT_TYPE_TCP;
            break;

        case 2:
            pData->l4port_type = L4PORT_TYPE_UDP;
            break;

        case 3:
            pData->l4port_type = L4PORT_TYPE_TCPORUDP;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L4 port type value error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x", *pData);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckL4Port_get */

/* Function Name:
 *      dal_esw_pie_rangeCheckL4Port_set
 * Description:
 *      Set the configuration of L4 port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of L4 port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckL4Port_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_l4Port_t *pData)
{
    int32   ret;
    uint32  value = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d, pData=%x", unit, index, pData);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_L4PORT(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    /* input value range check */
    if ((pData->dest_port > DAL_ESW_PIE_BIT_VALUE_MAX(1)))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "value error");
        return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    value = pData->upper_bound;
    if ((ret = reg_field_write(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_PORT_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    value = pData->lower_bound;    
    if ((ret = reg_field_write(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_PORT_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    switch (pData->dest_port)
    {
        case L4PORT_DIRECTION_SRC:
            value = 0;
            break;

        case L4PORT_DIRECTION_DST:
            value = 1;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L4 port type value error");
            return RT_ERR_FAILED;
    }
    if ((ret = reg_field_write(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_DESTPORTf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    switch (pData->l4port_type)
    {
        case L4PORT_TYPE_INVALID:
            value = 0;
            break;

        case L4PORT_TYPE_TCP:
            value = 1;
            break;

        case L4PORT_TYPE_UDP:
            value = 2;
            break;

        case L4PORT_TYPE_TCPORUDP:
            value = 3;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "L4 port type value error");
            return RT_ERR_FAILED;
    }
    if ((ret = reg_field_write(unit, (ESW_L4PORT_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckL4Port_set */

/* Function Name:
 *      dal_esw_pie_rangeCheckVid_get
 * Description:
 *      Get the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of VID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckVid_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d", unit, index);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_VID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pData->vid_type = VID_TYPE_INVALID;
            break;

        case 1:
            pData->vid_type = VID_TYPE_INNER;
            break;

        case 2:
            pData->vid_type = VID_TYPE_OUTER;
            break;

        case 3:
            pData->vid_type = VID_TYPE_INNEROROUTER;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "VID type value error");
            return RT_ERR_FAILED;
    }
    
    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_SRCPORT_NUMf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->src_port_num = value;    
    
    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_INNER_VID_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ivid_upper_bound = value;    

    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_INNER_VID_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ivid_lower_bound = value;    

    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_OUTER_VID_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ovid_upper_bound = value;    

    if ((ret = reg_field_read(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_OUTER_VID_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ovid_lower_bound = value;    
    
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x", *pData);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckVid_get */

/* Function Name:
 *      dal_esw_pie_rangeCheckVid_set
 * Description:
 *      Set the configuration of VID range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckVid_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_vid_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d, pData=%x", unit, index, pData);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_VID(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    switch (pData->vid_type)
    {
        case VID_TYPE_INVALID:
            value = 0;
            break;

        case VID_TYPE_INNER:
            value = 1;
            break;

        case VID_TYPE_OUTER:
            value = 2;
            break;

        case VID_TYPE_INNEROROUTER:
            value = 3;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "VID type value error");
            return RT_ERR_FAILED;
    }    
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    value = pData->src_port_num;
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_SRCPORT_NUMf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
   
    value = pData->ivid_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_INNER_VID_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    value = pData->ivid_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_0r + index*2), ESW_INNER_VID_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
   
    value = pData->ovid_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_OUTER_VID_UPPER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
   
    value = pData->ovid_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_VID_RANGE_CHECKING_TABLE_ENTRY0_1r + index*2), ESW_OUTER_VID_LOWER_BOUNDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckVid_set */

/* Function Name:
 *      dal_esw_pie_rangeCheckIp_get
 * Description:
 *      Get the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of IP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckIp_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d", unit, index);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_IP(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_0r + index*9), ESW_IP_ADDR0f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip4_lower_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_1r + index*9), ESW_IP_ADDR1f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip4_upper_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_2r + index*9), ESW_IP_ADDR2f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip3_lower_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_3r + index*9), ESW_IP_ADDR3f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip3_upper_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_4r + index*9), ESW_IP_ADDR4f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip2_lower_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_5r + index*9), ESW_IP_ADDR5f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip2_upper_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_6r + index*9), ESW_IP_ADDR6f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip1_lower_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_7r + index*9), ESW_IP_ADDR7f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->ip1_upper_bound = value;  

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_8r + index*9), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    switch (value)
    {
        case 0:
            pData->ip_type = IP_TYPE_IPV4_SRC;
            break;

        case 1:
            pData->ip_type = IP_TYPE_IPV4_DST;
            break;

        case 2:
            pData->ip_type = IP_TYPE_IPV6_SRC;
            break;

        case 3:
            pData->ip_type = IP_TYPE_IPV6_DST;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "IP type value error");
            return RT_ERR_FAILED;
    }    

    if ((ret = reg_field_read(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_8r + index*9), ESW_VALIDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->valid_mask = value;  

    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x", *pData);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckIp_get */

/* Function Name:
 *      dal_esw_pie_rangeCheckIp_set
 * Description:
 *      Set the configuration of IP range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of IP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckIp_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_ip_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d, pData=%x", unit, index, pData);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_IP(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    value = pData->ip4_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_0r + index*9), ESW_IP_ADDR0f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    value = pData->ip4_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_1r + index*9), ESW_IP_ADDR1f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
 
    value = pData->ip3_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_2r + index*9), ESW_IP_ADDR2f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
  
    value = pData->ip3_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_3r + index*9), ESW_IP_ADDR3f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
 
    value = pData->ip2_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_4r + index*9), ESW_IP_ADDR4f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
  
    value = pData->ip2_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_5r + index*9), ESW_IP_ADDR5f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
 
    value = pData->ip1_lower_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_6r + index*9), ESW_IP_ADDR6f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
  
    value = pData->ip1_upper_bound;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_7r + index*9), ESW_IP_ADDR7f, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
  
    switch (pData->ip_type)
    {
        case IP_TYPE_IPV4_SRC:
            value = 0;
            break;

        case IP_TYPE_IPV4_DST:
            value = 1;
            break;

        case IP_TYPE_IPV6_SRC:
            value = 2;
            break;

        case IP_TYPE_IPV6_DST:
            value = 3;
            break;

        default:
            PIE_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "IP type value error");
            return RT_ERR_FAILED;
    }        
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_8r + index*9), ESW_TYPEf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
 
    value = pData->valid_mask;
    if ((ret = reg_field_write(unit, (ESW_IP_RANGE_CHECKING_TABLE_ENTRY0_8r + index*9), ESW_VALIDf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckIp_set */

/* Function Name:
 *      dal_esw_pie_rangeCheckSrcPort_get
 * Description:
 *      Get the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 * Output:
 *      pData - configuration of source port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckSrcPort_get(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d", unit, index);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_SRCPORT(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_SOURCE_PORT_MASK_TABLE_ENTRYr, index, REG_ARRAY_INDEX_NONE, ESW_SRCPORTMASKf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    pData->src_port_mask = value;  
    
    PIE_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pData=%x", *pData);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckSrcPort_get */

/* Function Name:
 *      dal_esw_pie_rangeCheckSrcPort_set
 * Description:
 *      Set the configuration of source port range check.
 * Input:
 *      unit  - unit id
 *      index - entry index
 *      pData - configuration of source port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_rangeCheckSrcPort_set(uint32 unit, rtk_pie_id_t index, rtk_pie_rangeCheck_srcPortMask_t *pData)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, index=%d, pData=%x", unit, index, pData);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK((index >= HAL_MAX_NUM_OF_RANGE_CHECK_SRCPORT(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    value = pData->src_port_mask;
    if ((ret = reg_array_field_write(unit, ESW_SOURCE_PORT_MASK_TABLE_ENTRYr, index, REG_ARRAY_INDEX_NONE, ESW_SRCPORTMASKf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_rangeCheckSrcPort_set */

/* Function Name:
 *      dal_esw_pie_fieldSelectorEnable_get
 * Description:
 *      Get the enable status of field selector on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      fs_idx  - field selector index
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_fieldSelectorEnable_get(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value = 0;
    uint32  field = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, fs_idx=%d", unit, port, fs_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((fs_idx >= HAL_MAX_NUM_OF_FIELD_SELECTOR(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    
    switch (fs_idx)
    {    
        case 0:
            field = ESW_PFS0_ENf;
            break;
        case 1:
            field = ESW_PFS1_ENf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field selector index error");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_ENABLE_CONTROLr, port, REG_ARRAY_INDEX_NONE, field, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_fieldSelectorEnable_get */

/* Function Name:
 *      dal_esw_pie_fieldSelectorEnable_set
 * Description:
 *      Set the enable status of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_INPUT        - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_fieldSelectorEnable_set(uint32 unit, rtk_port_t port, rtk_pie_id_t fs_idx, rtk_enable_t enable)
{
    int32   ret;
    uint32  value = 0;
    uint32  field = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, fs_idx=%d, enable=%d", unit, port, fs_idx, enable);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((fs_idx >= HAL_MAX_NUM_OF_FIELD_SELECTOR(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_OUT_OF_RANGE);
    
    switch (fs_idx)
    {    
        case 0:
            field = ESW_PFS0_ENf;
            break;
        case 1:
            field = ESW_PFS1_ENf;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Field selector index error");
            return RT_ERR_FAILED;
    }

    /* chip's value translate */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
        
    PIE_SEM_LOCK(unit);

    /* get value from CHIP */
    if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_ENABLE_CONTROLr, port, REG_ARRAY_INDEX_NONE, field, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_fieldSelectorEnable_set */

/* Function Name:
 *      dal_esw_pie_fieldSelectorContent_get
 * Description:
 *      Get the content of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 * Output:
 *      pFs    - configuration of field selector.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_fieldSelectorContent_get(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    int32   ret;
    uint32  fs_sp = 0;
    uint32  fs_off0 = 0;
    uint32  fs_off1 = 0;
    uint32  fs_off2 = 0;
    uint32  fs_off3 = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, fs_idx=%d", unit, port, fs_idx);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((fs_idx >= HAL_MAX_NUM_OF_FIELD_SELECTOR(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pFs), RT_ERR_NULL_POINTER);
        
    PIE_SEM_LOCK(unit);
    if (0 == fs_idx)
    {
        /* get value from CHIP */
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_SPf, &fs_sp)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF0f, &fs_off0)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF1f, &fs_off1)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF2f, &fs_off2)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF3f, &fs_off3)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    else if (1 == fs_idx)
    {
        /* get value from CHIP */
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_SPf, &fs_sp)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF0f, &fs_off0)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF1f, &fs_off1)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF2f, &fs_off2)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_read(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF3f, &fs_off3)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (fs_sp)
    {
        case 0x0: /*0b000*/
            pFs->start = RAW;
            break;
        case 0x1: /*0b001*/
            pFs->start = DOT1Q;
            break;
        case 0x2: /*0b010*/
            pFs->start = LLC;
            break;
        case 0x3: /*0b011*/
            pFs->start = IPV4;
            break;
        case 0x4: /*0b100*/
            pFs->start = ARP;
            break;
        case 0x5: /*0b101*/
            pFs->start = IPV6;
            break;
        case 0x6: /*0b110*/
            pFs->start = TCP_UDP_ICMP_IGMP_ICMPV6;
            break;
        case 0x7: /*0b111*/
            pFs->start = TCP_UDP_PAYLOAD;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Start offset value error");
            return RT_ERR_FAILED;
    }
    pFs->offset0 = fs_off0;
    pFs->offset1 = fs_off1;
    pFs->offset2 = fs_off2;
    pFs->offset3 = fs_off3;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pFs=%d", *pFs);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_fieldSelectorContent_get */

/* Function Name:
 *      dal_esw_pie_fieldSelectorContent_set
 * Description:
 *      Set the content of field selector on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      fs_idx - field selector index
 *      pFs    - configuration of field selector.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_ENTRY_INDEX  - invalid entry index
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_fieldSelectorContent_set(
    uint32                       unit,
    rtk_port_t                   port,
    rtk_pie_id_t                 fs_idx,
    rtk_pie_fieldSelector_data_t *pFs)
{
    int32   ret;
    uint32  fs_sp = 0;
    uint32  fs_off0 = 0;
    uint32  fs_off1 = 0;
    uint32  fs_off2 = 0;
    uint32  fs_off3 = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, fs_idx=%d, pFs=%x", unit, port, fs_idx, pFs);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((fs_idx >= HAL_MAX_NUM_OF_FIELD_SELECTOR(unit)), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((NULL == pFs), RT_ERR_NULL_POINTER);

    /* chip's value translate */
    switch (pFs->start)
    {
        case RAW: 
            fs_sp = 0x0; /*0b000*/
            break;
        case DOT1Q: 
            fs_sp = 0x1;/*0b001*/
            break;
        case LLC: 
            fs_sp = 0x2;/*0b010*/
            break;
        case IPV4: 
            fs_sp = 0x3;/*0b011*/
            break;
        case ARP: 
            fs_sp = 0x4;/*0b100*/
            break;
        case IPV6: 
            fs_sp = 0x5;/*0b101*/
            break;
        case TCP_UDP_ICMP_IGMP_ICMPV6: 
            fs_sp = 0x6;/*0b110*/
            break;
        case TCP_UDP_PAYLOAD: 
            fs_sp = 0x7;/*0b111*/
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "Start offset value error");
            return RT_ERR_FAILED;
    }
    fs_off0 = pFs->offset0;
    fs_off1 = pFs->offset1;
    fs_off2 = pFs->offset2;
    fs_off3 = pFs->offset3;
        
    PIE_SEM_LOCK(unit);
    
    if (0 == fs_idx)
    {
        /* get value from CHIP */
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_SPf, &fs_sp)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF0f, &fs_off0)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF1f, &fs_off1)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF2f, &fs_off2)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS0_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS0_FOFF3f, &fs_off3)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    else if (1 == fs_idx)
    {
        /* get value from CHIP */
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_SPf, &fs_sp)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF0f, &fs_off0)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL0r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF1f, &fs_off1)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF2f, &fs_off2)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    
        if ((ret = reg_array_field_write(unit, ESW_PORT_FIELD_SELECTOR_FS1_CONTROL1r, port, REG_ARRAY_INDEX_NONE, ESW_FS1_FOFF3f, &fs_off3)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_fieldSelectorContent_set */

/*Currently, Pattern Match Function only implement on giga port (port24-27)*/
/* Function Name:
 *      dal_esw_pie_patternMatchEnable_get
 * Description:
 *      Get the enable status of pattern match on specific port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_pie_patternMatchEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d", unit, port);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    /*Currently, Pattern Match Function only implement on giga port (port24-27)*/
    RT_PARAM_CHK((port < HAL_PATTERN_MATCH_PORT_MIN(unit) || port > HAL_PATTERN_MATCH_PORT_MAX(unit)), RT_ERR_PORT_ID);

    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    if ((ret = reg_array_field_read(unit, ESW_PORT_PATTERN_MATCH_ENABLE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PPM_ENf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    /* chip's value translate */
    switch (value)
    {
        case 0:
            *pEnable = DISABLED;
            break;
        case 1:
            *pEnable = ENABLED;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_patternMatchEnable_get */

/* Function Name:
 *      dal_esw_pie_patternMatchEnable_set
 * Description:
 *      Set the enable status of pattern match on specific port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_pie_patternMatchEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  value = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, enable=%d", unit, port, enable);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable >= RTK_ENABLE_END, RT_ERR_OUT_OF_RANGE);
    /*Currently, Pattern Match Function only implement on giga port (port24-27)*/
    RT_PARAM_CHK((port < HAL_PATTERN_MATCH_PORT_MIN(unit) || port > HAL_PATTERN_MATCH_PORT_MAX(unit)), RT_ERR_PORT_ID);

    /* chip's value translate */
    switch (enable)
    {
        case DISABLED:
            value = 0;
            break;
        case ENABLED:
            value = 1;
            break;
        default:
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PIE), "");
            return RT_ERR_FAILED;
    }

    PIE_SEM_LOCK(unit);
    
    /* set value to CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_PATTERN_MATCH_ENABLE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PPM_ENf, &value)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_patternMatchEnable_set */

/* Function Name:
 *      dal_esw_pie_patternMatchContent_get
 * Description:
 *      Get the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pContent - content of pattern match.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
int32
dal_esw_pie_patternMatchContent_get(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    int32   ret;
    uint32  value = 0;
    uint32  pm_idx;
    uint32  start_reg = 0;
    uint32  last_reg = 0;
    uint32  care_reg = 0;
            
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d", unit, port);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);
    /*Currently, Pattern Match Function only implement on giga port (port24-27)*/
    RT_PARAM_CHK((port < HAL_PATTERN_MATCH_PORT_MIN(unit) || port > HAL_PATTERN_MATCH_PORT_MAX(unit)), RT_ERR_PORT_ID);

    PIE_SEM_LOCK(unit);
    
    /* get value from CHIP */
    for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++)
    {
        if ((ret = reg_array_field_read(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_CONTROLr, port, pm_idx, ESW_PM_CHARf, &value)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
        pContent->data[pm_idx].character = value;
    } /* end of for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++) */

    if ((ret = reg_array_field_read(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_START_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_START31_0f, &start_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_LAST_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_LAST31_0f, &last_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    if ((ret = reg_array_field_read(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_CARE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_CARE31_0f, &care_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }
    
    PIE_SEM_UNLOCK(unit);

    for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++)
    {
        pContent->data[pm_idx].start_bit = ((start_reg >> pm_idx) & 0x1);
        pContent->data[pm_idx].last_bit = ((last_reg >> pm_idx) & 0x1);   
        pContent->data[pm_idx].care_bit = ((care_reg >> pm_idx) & 0x1);   
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "pContent=%x", *pContent);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_patternMatchContent_get */

/* Function Name:
 *      dal_esw_pie_patternMatchContent_set
 * Description:
 *      Set the content of pattern match on specific port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pContent - content of pattern match.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If you want to check pattern "integer" need to:
 *      put 'r' into data[0], 'e' into data[1], 'g' into data[2] and so on
 *      turn on last_bit of data[0], turn on start_bit of data[6], turn on care_bit of data[0]~data[6].
 */
int32
dal_esw_pie_patternMatchContent_set(
    uint32                           unit,
    rtk_port_t                       port,
    rtk_pie_patternMatch_content_t   *pContent)
{
    int32   ret;
    uint32  value = 0;
    uint32  pm_idx;
    uint32  start_reg = 0;
    uint32  last_reg = 0;
    uint32  care_reg = 0;
        
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PIE), "unit=%d, port=%d, pContent=%x", unit, port, pContent);
    
    /* Check init state */
    RT_INIT_CHK(pie_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pContent), RT_ERR_NULL_POINTER);
    /*Currently, Pattern Match Function only implement on giga port (port24-27)*/
    RT_PARAM_CHK((port < HAL_PATTERN_MATCH_PORT_MIN(unit) || port > HAL_PATTERN_MATCH_PORT_MAX(unit)), RT_ERR_PORT_ID);
 
    start_reg = 0; last_reg = 0; care_reg = 0;
    for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++)
    {
        start_reg |= (pContent->data[pm_idx].start_bit & 0x1) << pm_idx;
        last_reg |= (pContent->data[pm_idx].last_bit & 0x1) << pm_idx;
        care_reg |= (pContent->data[pm_idx].care_bit & 0x1) << pm_idx;
    }

    PIE_SEM_LOCK(unit);

    /* set value to CHIP */
    for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++)
    {
        value = pContent->data[pm_idx].character;
        if ((ret = reg_array_field_write(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_CONTROLr, port, pm_idx, ESW_PM_CHARf, &value)) != RT_ERR_OK)
        {
            PIE_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
            return ret;
        }
    } /* end of for (pm_idx = 0; pm_idx < HAL_MAX_NUM_OF_PATTERN_MATCH_DATA(unit); pm_idx++)*/
    
    if ((ret = reg_array_field_write(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_START_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_START31_0f, &start_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_LAST_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_LAST31_0f, &last_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }

    if ((ret = reg_array_field_write(unit, ESW_PORT_PATTERN_MATCH_CHARACTER_CARE_CONTROLr, port, REG_ARRAY_INDEX_NONE, ESW_PM_CARE31_0f, &care_reg)) != RT_ERR_OK)
    {
        PIE_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PIE), "");
        return ret;
    }    
    
    PIE_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_esw_pie_patternMatchContent_set */

