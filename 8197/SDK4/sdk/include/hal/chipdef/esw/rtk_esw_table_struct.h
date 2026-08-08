
/* Superset Table Enum */
typedef enum rtk_esw_table_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACL_COUNTERt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBLt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDXt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDXt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCRt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ENTRY_HIT_INDI_STAt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_FORWARDINGt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDXt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDXt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIEt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIEt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICERt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPTt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_VLANt,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR0t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR1t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR2t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR3t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR4t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR5t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR6t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR7t,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR8t,
#endif

    ESW_TABLE_LIST_END,
} rtk_esw_table_list_t;


/* Internal Table Enum */
typedef enum rtk_int_esw_table_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_ACL_COUNTER_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_ACTTBL_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_CAM_ISFTIDX_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_CAM_ISNOTFTIDX_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_CBCCR_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_ENTRY_HIT_INDI_STA_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_FORWARDING_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_L2_ISFTIDX_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_L2_ISNOTFTIDX_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_MOVE_PIE_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_PIE_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_POLICER_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_SPT_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_VLAN_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR0_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR1_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR2_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR3_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR4_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR5_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR6_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR7_RTL8328,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INT_ESW_TSSCR8_RTL8328,
#endif

    INT_ESW_TABLE_LIST_END,
} rtk_int_esw_table_list_t;


typedef enum rtk_esw_acl_counterField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACL_COUNTER_ACL_COUNTER_64_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACL_COUNTER_ACL_COUNTER_64_1f,
#endif

    ESW_ACL_COUNTERFIELD_LIST_END,
} rtk_esw_acl_counterField_list_t;


typedef enum rtk_esw_acttblField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_HITINDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_STATISTICSf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_CP2CPUf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_DROPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_OUTTAGOPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_REDIRf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_INTAGOPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_PRIf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_DSCPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_POLICE_OUTRMKf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_MIRRORf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_AIF1f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_AIF2f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_AIF3f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_AIF4f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ACTTBL_AIF5f,
#endif

    ESW_ACTTBLFIELD_LIST_END,
} rtk_esw_acttblField_list_t;


typedef enum rtk_esw_cam_isftidxField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_AED_59_28f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_AED_27_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_AETf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_ISFTIDXf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISFTIDX_FTIDXf,
#endif

    ESW_CAM_ISFTIDXFIELD_LIST_END,
} rtk_esw_cam_isftidxField_list_t;


typedef enum rtk_esw_cam_isnotftidxField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_AED_59_28f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_AED_27_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_AETf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_ISFTIDXf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_PORTNUMf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_SBLKf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_DOT1XMACAUTHf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_ISSTATICf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_AGINGf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CAM_ISNOTFTIDX_SUSPENDf,
#endif

    ESW_CAM_ISNOTFTIDXFIELD_LIST_END,
} rtk_esw_cam_isnotftidxField_list_t;


typedef enum rtk_esw_cbccrField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCR_CLR_ACTTBLf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCR_CLR_PIEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCR_ENTRY_STARTf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCR_ENTRY_ENDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_CBCCR_STOPALEf,
#endif

    ESW_CBCCRFIELD_LIST_END,
} rtk_esw_cbccrField_list_t;


typedef enum rtk_esw_entry_hit_indi_staField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_ENTRY_HIT_INDI_STA_ELKNHIT_31_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ENTRY_HIT_INDI_STA_ELKNHIT_63_32f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ENTRY_HIT_INDI_STA_ELKNHIT_95_64f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_ENTRY_HIT_INDI_STA_ELKNHIT_127_96f,
#endif

    ESW_ENTRY_HIT_INDI_STAFIELD_LIST_END,
} rtk_esw_entry_hit_indi_staField_list_t;


typedef enum rtk_esw_forwardingField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_FORWARDING_PMf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_FORWARDING_CRSVLANf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_FORWARDING_AGINGf,
#endif

    ESW_FORWARDINGFIELD_LIST_END,
} rtk_esw_forwardingField_list_t;


typedef enum rtk_esw_l2_isftidxField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_AED_47_16f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_AED_15_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_AETf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_ISFTIDXf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISFTIDX_FTIDXf,
#endif

    ESW_L2_ISFTIDXFIELD_LIST_END,
} rtk_esw_l2_isftidxField_list_t;


typedef enum rtk_esw_l2_isnotftidxField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_AED_47_16f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_AED_15_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_AETf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_ISFTIDXf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_PORTNUMf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_SBLKf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_DOT1XMACAUTHf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_ISSTATICf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_NHf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_AGINGf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_L2_ISNOTFTIDX_SUSPENDf,
#endif

    ESW_L2_ISNOTFTIDXFIELD_LIST_END,
} rtk_esw_l2_isnotftidxField_list_t;


typedef enum rtk_esw_move_pieField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_ENTRY_LENGTHf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_ACTIONf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_MOVE_RULEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_MOVE_ACLACTTBLf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_STOPALEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_DIRECTIONf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_MOVE_TOf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_MOVE_PIE_MOVE_FROMf,
#endif

    ESW_MOVE_PIEFIELD_LIST_END,
} rtk_esw_move_pieField_list_t;


typedef enum rtk_esw_pieField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_DATA_31_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_DATA_63_32f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_DATA_95_64f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_DATA_127_96f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_VALIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_DATA_143_128f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_CAREBIT_31_0f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_CAREBIT_63_32f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_CAREBIT_95_64f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_CAREBIT_127_96f,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_PIE_CAREBIT_143_128f,
#endif

    ESW_PIEFIELD_LIST_END,
} rtk_esw_pieField_list_t;


typedef enum rtk_esw_policerField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_PBSf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_CBSf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_TPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_TCf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_PIRf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_CIRf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_YEL_DPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_RED_DPf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_COLORAWAREf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_POLICER_TYPEf,
#endif

    ESW_POLICERFIELD_LIST_END,
} rtk_esw_policerField_list_t;


typedef enum rtk_esw_sptField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT15_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT14_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT13_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT12_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT11_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT10_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT9_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT8_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT7_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT6_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT5_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT4_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT3_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT2_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT1_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT0_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT28_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT27_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT26_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT25_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT24_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT23_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT22_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT21_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT20_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT19_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT18_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT17_SPT_STATEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_SPT_PORT16_SPT_STATEf,
#endif

    ESW_SPTFIELD_LIST_END,
} rtk_esw_sptField_list_t;


typedef enum rtk_esw_vlanField_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_VLAN_FIDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_VLAN_VLAN_BASE_FWDf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_VLAN_UTAGf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_VLAN_MBRf,
#endif

    ESW_VLANFIELD_LIST_END,
} rtk_esw_vlanField_list_t;


typedef enum rtk_esw_tsscr0Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR0_BC_RATEf,
#endif

    ESW_TSSCR0FIELD_LIST_END,
} rtk_esw_tsscr0Field_list_t;


typedef enum rtk_esw_tsscr1Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR1_BC_EXCSTf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR1_BC_COUNTERf,
#endif

    ESW_TSSCR1FIELD_LIST_END,
} rtk_esw_tsscr1Field_list_t;


typedef enum rtk_esw_tsscr2Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR2_BC_ENABLEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR2_BC_BUNITf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR2_BC_BURSTf,
#endif

    ESW_TSSCR2FIELD_LIST_END,
} rtk_esw_tsscr2Field_list_t;


typedef enum rtk_esw_tsscr3Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR3_MC_RATEf,
#endif

    ESW_TSSCR3FIELD_LIST_END,
} rtk_esw_tsscr3Field_list_t;


typedef enum rtk_esw_tsscr4Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR4_MC_EXCSTf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR4_MC_COUNTERf,
#endif

    ESW_TSSCR4FIELD_LIST_END,
} rtk_esw_tsscr4Field_list_t;


typedef enum rtk_esw_tsscr5Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR5_MC_TYPEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR5_MC_ENABLEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR5_MC_BUNITf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR5_MC_BURSTf,
#endif

    ESW_TSSCR5FIELD_LIST_END,
} rtk_esw_tsscr5Field_list_t;


typedef enum rtk_esw_tsscr6Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR6_UNUA_RATEf,
#endif

    ESW_TSSCR6FIELD_LIST_END,
} rtk_esw_tsscr6Field_list_t;


typedef enum rtk_esw_tsscr7Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR7_UNUA_EXCSTf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR7_UNUA_COUNTERf,
#endif

    ESW_TSSCR7FIELD_LIST_END,
} rtk_esw_tsscr7Field_list_t;


typedef enum rtk_esw_tsscr8Field_list_e
{
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR8_UNUA_ENABLEf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR8_UNUA_BUNITf,
#endif
#if defined(CONFIG_SDK_RTL8328)
    ESW_TSSCR8_UNUA_BURSTf,
#endif

    ESW_TSSCR8FIELD_LIST_END,
} rtk_esw_tsscr8Field_list_t;

extern rtk_table_t rtk_esw_table_list[];
