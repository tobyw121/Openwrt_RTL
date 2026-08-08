#ifndef __MS_DRAM_PARA_H
#define __MS_DRAM_PARA_H
#include "dram_device_info.h" 

#define PHY_ADDR_CMD_LATENCY 7
#define PHY_WRITE_DATA_LATENCY 6
#define PHY_DQS_LATENCY 8               //maybe should read phy ip register 0x244
#define PHY_READ_LATENCY 1             // PHY_DQS_LATENCY - PHY_ADDR_CMD_LATENCY

//===================DRAM_INFO=========================
const struct dram_info ddr1_dev = {
  DDR_1,  
  PAGE_1K, 
  BANK_4, 
  HALF_DQ32
}; // ddr1_dev

const struct dram_info ddr2_dram_M14D5121632A_dev = {
  DDR_2,  
  PAGE_2K, 
  BANK_4, 
  HALF_DQ32
};


//=================DRAM MODE REGISTER=====================

const struct dram_mode_reg_info ddr2_800_dram_M14D5121632A_mode_reg =  {
  BST_LEN_8,
  SENQUENTIAL,
  0x7, // mode0_cas: 7
  0x0, // mode0_wr
  0,   // mode1_dll_en_n
  0,   // mode1_ocd;      //1 -> 70%, 0 -> 100% Brian: 100%
  0x10, // mode1_odt;    //0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat
  0,    // mode2_cwl
  0,   // parity_lat
  6,   // cr_mrinfo_wr_lat
  7,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat
};

const struct dram_mode_reg_info ddr2_1066_dram_M14D5121632A_mode_reg = {
  BST_LEN_8, 
  SENQUENTIAL,
  0x7, // mode0_cas: 7
  0x0, // mode0_wr 
  0,   // mode1_dll_en_n  
  0,   // mode1_ocd;      //1 -> 70%, 0 -> 100%
  0x10, // mode1_odt;    //0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat  
  0,    // mode2_cwl   
  0,   // parity_lat
  6,   // cr_mrinfo_wr_lat
  7,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat  
};

const struct dram_mode_reg_info ddr2_1120_dram_M14D5121632A_mode_reg_winbond = 
{
  BST_LEN_8, 
  SENQUENTIAL,
  0x9, // mode0_cas: 7
  0x0, // mode0_wr 
  0,   // mode1_dll_en_n  
  0,   // mode1_ocd;      //1 -> 70%, 0 -> 100%
  0x10, // mode1_odt;    //0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat  
  0,    // mode2_cwl   
  0,   // parity_lat
  8,   // cr_mrinfo_wr_lat
  9,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat  
};

const struct dram_mode_reg_info ddr1_mode_reg_400mhz = {
  BST_LEN_8, 
  SENQUENTIAL,
  0x3, // mode0_cas: 3 
  0x0, // mode0_wr 
  0,   // mode1_dll_en_n  
  1,   // mode1_ocd;	  //1 -> 70%, 0 -> 100%
  0x1, // mode1_odt;	//0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat=0
  0,   // mode2_cwl:5
  0,   // parity_lat
  1,   // cr_mrinfo_wr_lat
  3,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat  
}; // ddr1_mode_reg_400mhz

const struct dram_mode_reg_info ddr1_mode_reg_500mhz = {
  BST_LEN_8, 
  SENQUENTIAL,
  0x3, // mode0_cas: 3 
  0x0, // mode0_wr 
  0,   // mode1_dll_en_n 
  1,   // mode1_ocd;	  //1 -> 70%, 0 -> 100%
  0x1, // mode1_odt;	//0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat=0
  0,   // mode2_cwl:5
  0,   // parity_lat
  1,   // cr_mrinfo_wr_lat
  3,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat  
}; // ddr1_mode_reg_500mhz

//================DRAM Timing info=======================
const struct dram_timing_info ddr1_timing_400mhz = {
  DRAM_512M_TRFC,      // trfc_ps; 70000
  69999800,   // trefi_ps;
  3,          // wr_max_tck;
  15000,      // trcd_ps; 
  15000,      // trp_ps;
  40000,      // tras_ps;
  2,          // trrd_tck;
  15000,       // twr_ps;
  2,          // twtr_tck; 
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  0,          // trtp_tck;
  1,          // tccd_tck;
  55000,      // trc_ps;
  0,		  // tccd_s_tck;
  0, 		  // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
}; // ddr1_timing_400mhz

const struct dram_timing_info ddr1_timing_500mhz = {
  DRAM_512M_TRFC,      // trfc_ps; 70000
  69999800,   // trefi_ps;
  3,          // wr_max_tck;
  15000,      // trcd_ps; 
  15000,      // trp_ps;
  40000,      // tras_ps;
  2,          // trrd_tck;
  15000,       // twr_ps;
  2,          // twtr_tck; 
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  0,          // trtp_tck;
  1,          // tccd_tck;
  55000,      // trc_ps;
  0,		  // tccd_s_tck;
  0, 		  // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
}; // ddr1_timing_400mhz

const struct dram_timing_info ddr2_800_dram_M14D5121632A_timing = {
  DRAM_512M_TRFC,      // trfc_ps;
  17000000,   // trefi_ps;     //todo 69999800
  8,          // wr_max_tck;    // todo
  15000,      // trcd_ps; 
  15000,      // trp_ps;
  45000,      // tras_ps;
  4,          // trrd_tck;          //todo
  15000,       // twr_ps;
  3,          // twtr_tck;          //todo
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  3,          // trtp_tck;          //todo
  2,          // tccd_tck;
  57500,       // trc_ps;
  0,    // tccd_s_tck;
  0,     // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
};

const struct dram_timing_info ddr2_1066_dram_M14D5121632A_timing = {
  DRAM_512M_TRFC,      // trfc_ps;
  17000000,   // trefi_ps;     //todo
  8,          // wr_max_tck;    // todo
  13125,      // trcd_ps; 
  13250,      // trp_ps;
  45000,      // tras_ps;
  6,          // trrd_tck;          //todo
  15000,       // twr_ps;
  4,          // twtr_tck;          //todo
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  4,          // trtp_tck;          //todo
  2,          // tccd_tck;
  58125,       // trc_ps;
  0,    // tccd_s_tck;
  0,     // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
};

const struct dram_timing_info ddr2_1066_dram_timing_APM32 = {
  DRAM_512M_TRFC,      // trfc_ps;
  17000000,   // trefi_ps;     //todo
  8,          // wr_max_tck;    // todo
  18000,      // trcd_ps; 
  18000,      // trp_ps;
  45000,      // tras_ps;
  6,          // trrd_tck;          //todo
  15000,       // twr_ps;
  4,          // twtr_tck;          //todo
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  4,          // trtp_tck;          //todo
  2,          // tccd_tck;
  58125,       // trc_ps;
  0,    // tccd_s_tck;
  0,     // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
};

const struct dram_timing_info ddr2_1120_dram_M14D5121632A_timing_winbond = 
{
  DRAM_512M_TRFC,      // trfc_ps;
  17000000,   // trefi_ps;     //todo
  10,          // wr_max_tck;    // todo
  13333,      // trcd_ps; 
  13333,      // trp_ps;
  45000,      // tras_ps;
  7,          // trrd_tck;          //todo
  15000,       // twr_ps;
  5,          // twtr_tck;          //todo
  //13090,      // trp_ps;
  2,          // tmrd_tck;
  5,          // trtp_tck;          //todo
  2,          // tccd_tck;
  58333,       // trc_ps;
  0,    // tccd_s_tck;
  0,     // twtr_s_tck;
  3,    //  tcke_tck;
  0,    // tzqcs_tck;
  50000       // tfaw_ps;
};

//==================PHY PARAMETER INFO==================
const struct dram_phy_ip_info ddr2_phy_mcm_1066 = {
  1,  // mck_default; winbond 6; etron 3
  0, // mdqs_default;  winbond 7; etron -15
  0xe, //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x3fc, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0xf, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x12111211,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x12111111,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x11121111,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x12111111,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x12121312,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x12121212,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x11121112,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x12121211,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x66666666,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x66666666,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x5444,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};
const struct dram_phy_ip_info ddr2_phy_mcm_800 = {
  1,  // mck_default; winbond 6; etron 3
  0, // mdqs_default;  winbond 7; etron -15
  0xe, //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x3fc, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0xf, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x12111211,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x12111111,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x11121111,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x12111111,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x12121312,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x12121212,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x11121112,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x12121211,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x66666666,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x66666666,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x5444,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};
const struct dram_phy_ip_info ddr2_phy_mcm_1120_winbond =  {
  -2,  // mck_default; winbond 6; etron 3
  0, // mdqs_default;  winbond 7; etron -15
  0xe, //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x3fc, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0xf, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x11111111,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x11111111,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x11111111,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x10111110,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x11121111,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x12111111,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x11121111,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x11111111,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x66666666,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x66666666,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x6464,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};
const struct dram_phy_ip_info ddr2_phy_mcm_1066_APM32MB =   {
 -7,  // mck_default; winbond 6; etron 3
  2, // mdqs_default;  winbond 7; etron -15
  0xe, //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x3fc, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0xf, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x11101210,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x11121011,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x12111111,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x11121111,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x12111211,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x12131013,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x12121212,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x11131212,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x44444444,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
 0x44444444,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x4444,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};

const struct dram_phy_ip_info ddr1_phy_mcm_400 = {
  6,  // mck_default; winbond 6; etron 3
  7, // mdqs_default;  winbond 7; etron -15
  0xe, //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x7f8, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0x0, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x0e0e0e0e,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x0e0e0e0e,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x0e0e0e0e,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x0e0e0e0e,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x0e0e0e0e,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x0e0e0e0e,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x0e0e0e0e,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x0e0e0e0e,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x55555555,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x55555555,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x55555555,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x55555555,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x66666666,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x66666666,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x6464,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};

const struct dram_phy_ip_info ddr1_phy_mcm_500 = {
  0,           // mck_default; 
  0,           // mdqs_default; 
  0xc,       //  dqs_en_default;
  0x9,       //  rx_fifo_default;
  0x3ff,     // odt_default;
  0x551,     // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0x00,     // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0x0,       // cmd_driving;   //nt,pt, 0x210
  0x0,       // ba_addr_driving;   //nt,pt, 0x214
  0x50,       //uint8_t dq_driving;   //nt,pt, 0x218
  0x10101010,         // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x10101010,         // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x10101010,         // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x10101010,         // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x15131413,         // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x11151314,         // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x15141617,         // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x17141616,         // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,         //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,         //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,         //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,         //bit 8 ~ 15, default 0x33333333, 0x280
  0x44444444,         // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x44444444,         // tx_delay_tap_1;  // bit 8 ~ 15, default 0x44444444
  0x4444,		// dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default 0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};

//==================DRAM INFO=========================

const struct dram_device_info ddr1_400_dram_info = { // reference Winbond W9425G6JH 4m * 4 banks * 16 bits
  &ddr1_dev,
  &ddr1_mode_reg_400mhz,
  &ddr1_timing_400mhz,
  &ddr1_phy_mcm_400,
  5000,          // ddr_period_ps, DDR= 400
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr1_500_dram_info = {  // reference Winbond W9425G6JH 4m * 4 banks * 16 bits
  &ddr1_dev,
  &ddr1_mode_reg_500mhz,
  &ddr1_timing_500mhz,
  &ddr1_phy_mcm_500,
  4000,          // ddr_period_ps, DDR= 500
  (enum     dfi_ratio_type     *) DFI_RATIO_1
};

const struct dram_device_info ddr2_800_dram_M14D5121632A_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_800_dram_M14D5121632A_mode_reg,
  &ddr2_800_dram_M14D5121632A_timing,
  &ddr2_phy_mcm_800,
  2500,          // ddr_period_ps, DDR2= 800
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_winbond_800_dram_M14D5121632A_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_800_dram_M14D5121632A_mode_reg,
  &ddr2_800_dram_M14D5121632A_timing,
  &ddr2_phy_mcm_1120_winbond,
  2500,          // ddr_period_ps, DDR2= 800
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_APM32MB_800_dram = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_800_dram_M14D5121632A_mode_reg,
  &ddr2_1066_dram_timing_APM32,
  &ddr2_phy_mcm_1066_APM32MB,
  2500,          // ddr_period_ps, DDR2= 800
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_1066_dram_M14D5121632A_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_1066_dram_M14D5121632A_mode_reg,
  &ddr2_1066_dram_M14D5121632A_timing,
  &ddr2_phy_mcm_1066,
  1818,          // ddr_period_ps, DDR2= 1066  1876
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_winbond_1066_dram_M14D5121632A_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_1120_dram_M14D5121632A_mode_reg_winbond,
  &ddr2_1120_dram_M14D5121632A_timing_winbond,
  &ddr2_phy_mcm_1120_winbond,
  1875,          // ddr_period_ps, DDR2= 1066  1876
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_winbond_1120_dram_M14D5121632A_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_1120_dram_M14D5121632A_mode_reg_winbond,
  &ddr2_1120_dram_M14D5121632A_timing_winbond,
  &ddr2_phy_mcm_1120_winbond,
  1785,          // ddr_period_ps, DDR2= 1066  1876
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_APM32MB_1066_dram_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_1066_dram_M14D5121632A_mode_reg,
  &ddr2_1066_dram_timing_APM32,
  &ddr2_phy_mcm_1066_APM32MB,
  1876,          // ddr_period_ps, DDR2= 1066  1876
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

/******************************************************
****
**** The below is for discret DDR2 memory
**** 
*******************************************************/
#ifdef CONFIG_DDR2_DISCRET
const struct dram_mode_reg_info ddr2_800_dram_discret_mode_reg = {
   BST_LEN_8,
   SENQUENTIAL,
   0x5, // mode0_cas: 5
   0x0, // mode0_wr
   0,   // mode1_dll_en_n
   0,   // mode1_ocd;      //1 -> 70%, 0 -> 100%
   0x10, // mode1_odt;    //0x1 -> 75,   0x10 -> 150,  0x11 -> 50
   0,   // mode1_all_lat
   0,    // mode2_cwl
   0,   // parity_lat
   4,   // cr_mrinfo_wr_lat
   5,   // cr_mrinfo_rd_lat
   0    // cr_mrinfo_add_lat
 };

const struct dram_mode_reg_info ddr2_1066_dram_discret_mode_reg = {
  BST_LEN_8, 
  SENQUENTIAL,
  0x7, // mode0_cas: 7
  0x0, // mode0_wr 
  0,   // mode1_dll_en_n  
  0,   // mode1_ocd;      //1 -> 70%, 0 -> 100%
  0x10, // mode1_odt;    //0x1 -> 75,   0x10 -> 150,  0x11 -> 50
  0,   // mode1_all_lat  
  0,    // mode2_cwl   
  0,   // parity_lat
  6,   // cr_mrinfo_wr_lat
  7,   // cr_mrinfo_rd_lat
  0    // cr_mrinfo_add_lat  
};

const struct dram_phy_ip_info ddr2_phy_discret_800 =  {
  6,  // mck_default; winbond 6; etron 3
  5, // mdqs_default;  winbond 7; etron -15
  0xf , //  dqs_en_default; 
  0xa, //  rx_fifo_default; 
  0x3fc, // odt_default;
  0xff0, // dqs_driving;   //nnt, npt, pnt,ppt, 0x25c
  0xff, // dck_driving;   //nnt, npt, pnt,ppt, 0x264
  0xf, // cmd_driving;   //nt,pt, 0x210
  0x0, // ba_addr_driving;   //nt,pt, 0x214
  0xf0, //uint8_t dq_driving;   //nt,pt, 0x218
  0x12101210,  // rx_delay_tap_0;  //bit 0 ~ 3, default 0x10101010, 0x28c
  0x10111211,  // rx_delay_tap_1;  //bit 4 ~ 7, default 0x10101010 , 0x29c
  0x11111211,  // rx_delay_tap_3;  //bit 8 ~ 11, default 0x10101010, 0x290
  0x12111111,  // rx_delay_tap_4;  //bit 12 ~ 15, default 0x10101010 , 0x2a0
  0x12111210,  // rx_delay_tap_0; //bit 0 ~ 3, default 0x10101010, 0x2ac
  0x11121211,  // rx_delay_tap_1; //bit 4 ~ 7, default 0x10101010 , 0x2bc
  0x12121313,  // rx_delay_tap_3; //bit 8 ~ 11, default 0x10101010, 0x2b0
  0x13131312,  // rx_delay_tap_4; //bit 12 ~ 15, default 0x10101010 , 0x2c0
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x26c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x270
  0x33333333,  //bit 0 ~ 7, default 0x33333333, 0x27c
  0x33333333,  //bit 8 ~ 15, default 0x33333333, 0x280
  0x44444444,  // tx_delay_tap_0;  // bit 0 ~ 7, default 0x44444444
  0x44444444,  // tx_delay_tap_1; // bit 8 ~ 15, default 0x44444444
  0x5444,  //dqs_dm_delay_tap; bit 15~0 dm: bit 7~4 bit 12~15 , default 0x4444  loca:0x240
  0x44444444, // bit 0 ~ 7, default 0x44444444, 0x200
  0x44444444, // bit 8 ~ 15, default 0x44444444, 0x204
  0x444, // bank adr 0 ~ 2, default 0x0444, 0x208
  0x444444, // CKE[3:0], RAS[7:4], CAS[11:8], WE[15:12], CS[19:16], ODT[23:20], default  0x00444444, 0x20c
  0x80200DF, // only for bit field => BIT_DCK_POWER_SEL[28:20], BIT_DCK_LDO_VSEL[19:18], BIT_DCK_DLY_SEL[8:4]
  0x11117777 // CAL_SHIFT_CTRL
};

const struct dram_device_info ddr2_800_dram_discret_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_800_dram_discret_mode_reg,
  &ddr2_800_dram_M14D5121632A_timing,
  &ddr2_phy_discret_800,
  2540,          // ddr_period_ps, DDR2= 800
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};

const struct dram_device_info ddr2_1066_dram_discret_info = {
  &ddr2_dram_M14D5121632A_dev,
  &ddr2_1066_dram_discret_mode_reg,
  &ddr2_1066_dram_M14D5121632A_timing,
  &ddr2_phy_discret_800,
  1876,          // ddr_period_ps, DDR2= 1066 1876 
  (enum     dfi_ratio_type     *)DFI_RATIO_1
};
#endif

#endif //_MS_DRAM_PARA_H
 
