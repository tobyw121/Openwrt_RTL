/*
 * Include Files
 */
#include <common/rt_autoconf.h>

/* Superset Table Enum */
typedef enum rtk_ssw_table_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_LOG_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_MULTICAST_INDEX_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_SVLAN_TABLEt,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_VLAN_TABLEt,
#endif

    SSW_TABLE_LIST_END,
} rtk_ssw_table_list_t;


/* Internal Table Enum */
typedef enum rtk_int_ssw_table_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_L2CAM_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_L2_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_LOG_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_MULTICAST_INDEX_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_PIE89_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_SVLAN_TABLE_RTL8389,
#endif
#if defined(CONFIG_SDK_RTL8389)
    INT_SSW_VLAN_TABLE_RTL8389,
#endif

    INT_SSW_TABLE_LIST_END,
} rtk_int_ssw_table_list_t;


typedef enum rtk_ssw_l2cam_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_FIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_M_CVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_SIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_MACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_DIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_IP_MULTIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_IP_CVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_AGEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_STATICf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_INDEXf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_AUTHf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_DABLOCKf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_SABLOCKf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2CAM_TABLE_SPAf,
#endif

    SSW_L2CAM_TABLEFIELD_LIST_END,
} rtk_ssw_l2cam_tableField_list_t;


typedef enum rtk_ssw_l2_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_SIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_MACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_DIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_IP_MULTIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_IP_CVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_AGEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_STATICf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_INDEXf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_AUTHf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_DABLOCKf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_SABLOCKf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_L2_TABLE_SPAf,
#endif

    SSW_L2_TABLEFIELD_LIST_END,
} rtk_ssw_l2_tableField_list_t;


typedef enum rtk_ssw_log_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_LOG_TABLE_PKTCNTf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_LOG_TABLE_BYTECNTf,
#endif

    SSW_LOG_TABLEFIELD_LIST_END,
} rtk_ssw_log_tableField_list_t;


typedef enum rtk_ssw_multicast_index_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_MULTICAST_INDEX_TABLE_PORTMSKf,
#endif

    SSW_MULTICAST_INDEX_TABLEFIELD_LIST_END,
} rtk_ssw_multicast_index_tableField_list_t;


typedef enum rtk_ssw_pie89_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_DMACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_SMACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_DIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_SIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWLABELf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PKTCVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_RVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_DSTPORTf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_SRCPORTf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_ETHTYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_IPPROTOf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_TOSf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PKTCPRIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_SLPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_FRAMETYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PKTTAGIFf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_IPV6MLDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_IPV6f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_IPV4f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PKTSTAGIFf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PKTSPRIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_FLOWTABLE_PKTSVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_TCPFLAGf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_SLPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_FRAMETYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_IPV6MLDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_IPV6f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_IPV4f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_PATTERNMATCHf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_INACL_PKTSVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_ACTGROUPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_SRAM_17Bf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_SRAM_7Bf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_DMACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_SMACf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_DIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_SIPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWLABELf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PKTCVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_RVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_DSTPORTf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_SRCPORTf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_ETHTYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_IPPROTOf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_TOSf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PKTCPRIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_SLPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_FRAMETYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PKTTAGIFf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6MLDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV6f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_IPV4f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PKTSTAGIFf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PKTSPRIf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_FLOWTABLE_PKTSVIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_TCPFLAGf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_SLPf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_FRAMETYPEf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_IPV6MLDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_IPV6f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_IPV4f,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_PATTERNMATCHf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_PIE89_TABLE_CARE_INACL_PKTSVIDf,
#endif

    SSW_PIE89_TABLEFIELD_LIST_END,
} rtk_ssw_pie89_tableField_list_t;


typedef enum rtk_ssw_svlan_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_SVLAN_TABLE_SMBRf,
#endif

    SSW_SVLAN_TABLEFIELD_LIST_END,
} rtk_ssw_svlan_tableField_list_t;


typedef enum rtk_ssw_vlan_tableField_list_e
{
#if defined(CONFIG_SDK_RTL8389)
    SSW_VLAN_TABLE_MEMBER_SETf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_VLAN_TABLE_UNTAG_SETf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_VLAN_TABLE_FIDf,
#endif
#if defined(CONFIG_SDK_RTL8389)
    SSW_VLAN_TABLE_MSTIf,
#endif

    SSW_VLAN_TABLEFIELD_LIST_END,
} rtk_ssw_vlan_tableField_list_t;

extern rtk_table_t rtk_ssw_table_list[];

