set global_flash_type           spi_nor

## NOR SPI flash feature configuration
set nor_spi_num_chips           2
set nor_spi_prefer_divisor      16
set nor_spi_size_per_chip       16M
set nor_spi_prefer_rx_delay0    0
set nor_spi_prefer_rx_delay1    0
set nor_spi_prefer_rx_delay2    0
set nor_spi_prefer_rx_delay3    0

set nor_spi_prefer_rd_cmd       0xeb
set nor_spi_prefer_rd_cmd_io    SIO
set nor_spi_prefer_rd_dummy_c   6
set nor_spi_prefer_rd_addr_io   QIO
set nor_spi_prefer_rd_data_io   QIO

set nor_spi_wr_cmd              0x02
set nor_spi_wr_cmd_io           SIO
set nor_spi_wr_dummy_c          0
set nor_spi_wr_addr_io          SIO
set nor_spi_wr_data_io          SIO
set nor_spi_wr_boundary         256

set nor_spi_erase_cmd           0x20
set nor_spi_erase_unit          4K

set nor_spi_pm_method           RWSR
set nor_spi_pm_rdsr_cmd         0x05
set nor_spi_pm_rdsr2_cmd        0x00
set nor_spi_pm_wrsr_cmd         0x01
set nor_spi_pm_enable_bits      0x40
set nor_spi_pm_status_len       1

set nor_spi_rdbusy_cmd          0x05
set nor_spi_rdbusy_len          0x2
set nor_spi_rdbusy_loc          0
set nor_spi_rdbusy_polling_period   0

set nor_spi_id                  0xc22018

## PLL information
#set pll_gen2_set_by             pin
set pll_gen2_set_by             software
set pll_gen2_cpu_clock_mhz      750
set pll_gen2_dram_clock_mhz     200
set pll_gen2_lx_clock_mhz       200

## peripheral information
set peri_uart_baudrate          115200

## dram information
## MCR
set dram_gen2_IPREF                 0
set dram_gen2_DPREF                 0
## DCR
set dram_gen2_BANKCNT               2
set dram_gen2_DBUSWID               1 ;#0:8bit, 1:16bit
set dram_gen2_ROWCNT                2 ;# 16K
set dram_gen2_COLCNT                2
set dram_gen2_DCHIPSEL              0
set dram_gen2_FAST_RX               0
set dram_gen2_BSTREF                0
## DTR input
set dram_gen2_refi_ns               7800
set dram_gen2_rp_ns                 15
set dram_gen2_rcd_ns                15
set dram_gen2_ras_ns                45
set dram_gen2_rfc_ns                328
set dram_gen2_wr_ns                 15
set dram_gen2_rrd_ns                10
set dram_gen2_fawg_ns               50
set dram_gen2_wtr_ns                8
set dram_gen2_rtp_ns                8

## MPMR0
set dram_gen2_PM_MODE               0x0
set dram_gen2_T_CKE                 0xf
set dram_gen2_T_RSD                 0x3ff
set dram_gen2_T_XSREF               0x3ff
## MPMR1
set dram_gen2_T_XARD                0xf
set dram_gen2_T_AXPD                0xf
## DIDER
set dram_gen2_DQS0_EN_HCLK          0x0
set dram_gen2_DQS0_EN_TAP           0x0
set dram_gen2_DQS1_EN_HCLK          0x0
set dram_gen2_DQS1_EN_TAP           0x0
## D23OSCR
set dram_gen2_ODT_ALWAYS_ON         0x0
set dram_gen2_TE_ALWAYS_ON          0x0
## DACCR
set dram_gen2_AC_MODE               0x1
set dram_gen2_DQS_SE                0x0
set dram_gen2_DQS0_GROUP_TAP        0x0
set dram_gen2_DQS1_GROUP_TAP        0x0
set dram_gen2_AC_DYN_BPTR_CLR_EN    0x0
set dram_gen2_AC_BPTR_CLEAR         0x1
set dram_gen2_AC_DEBUG_SEL          0x0
## DACSPCR
set dram_gen2_AC_SILEN_PERIOD_EN    0x0
set dram_gen2_AC_SILEN_TRIG         0x0
set dram_gen2_AC_SILEN_PERIOD_UNIT  0x0
set dram_gen2_AC_SILEN_PERIOD       0x0
set dram_gen2_AC_SILEN_LEN          0x7f
## DACSPAR
set dram_gen2_AC_SPS_DQ15R          0
set dram_gen2_AC_SPS_DQ14R          0
set dram_gen2_AC_SPS_DQ13R          0
set dram_gen2_AC_SPS_DQ12R          0
set dram_gen2_AC_SPS_DQ11R          0
set dram_gen2_AC_SPS_DQ10R          0
set dram_gen2_AC_SPS_DQ9R           0
set dram_gen2_AC_SPS_DQ8R           0
set dram_gen2_AC_SPS_DQ7R           0
set dram_gen2_AC_SPS_DQ6R           0
set dram_gen2_AC_SPS_DQ5R           0
set dram_gen2_AC_SPS_DQ4R           0
set dram_gen2_AC_SPS_DQ3R           0
set dram_gen2_AC_SPS_DQ2R           0
set dram_gen2_AC_SPS_DQ1R           0
set dram_gen2_AC_SPS_DQ0R           0
set dram_gen2_AC_SPS_DQ15F          0
set dram_gen2_AC_SPS_DQ14F          0
set dram_gen2_AC_SPS_DQ13F          0
set dram_gen2_AC_SPS_DQ12F          0
set dram_gen2_AC_SPS_DQ11F          0
set dram_gen2_AC_SPS_DQ10F          0
set dram_gen2_AC_SPS_DQ9F           0
set dram_gen2_AC_SPS_DQ8F           0
set dram_gen2_AC_SPS_DQ7F           0
set dram_gen2_AC_SPS_DQ6F           0
set dram_gen2_AC_SPS_DQ5F           0
set dram_gen2_AC_SPS_DQ4F           0
set dram_gen2_AC_SPS_DQ3F           0
set dram_gen2_AC_SPS_DQ2F           0
set dram_gen2_AC_SPS_DQ1F           0
set dram_gen2_AC_SPS_DQ0F           0


set dram_gen2_static_cal_data_0     0x0F1E0F00
set dram_gen2_static_cal_data_1     0x0F1E0F00
set dram_gen2_static_cal_data_2     0x0F1E0F00
set dram_gen2_static_cal_data_3     0x0F1E0F00
set dram_gen2_static_cal_data_4     0x0F1E0F00
set dram_gen2_static_cal_data_5     0x0F1E0F00
set dram_gen2_static_cal_data_6     0x0F1E0F00
set dram_gen2_static_cal_data_7     0x0F1E0F00
set dram_gen2_static_cal_data_8     0x0F1E0F00
set dram_gen2_static_cal_data_9     0x0F1E0F00
set dram_gen2_static_cal_data_10    0x0F1E0F00
set dram_gen2_static_cal_data_11    0x0F1E0F00
set dram_gen2_static_cal_data_12    0x0F1E0F00
set dram_gen2_static_cal_data_13    0x0F1E0F00
set dram_gen2_static_cal_data_14    0x0F1E0F00
set dram_gen2_static_cal_data_15    0x0F1E0F00
set dram_gen2_static_cal_data_16    0x001E0F00
set dram_gen2_static_cal_data_17    0x001E0F00
set dram_gen2_static_cal_data_18    0x001E0F00
set dram_gen2_static_cal_data_19    0x001E0F00
set dram_gen2_static_cal_data_20    0x001E0F00
set dram_gen2_static_cal_data_21    0x001E0F00
set dram_gen2_static_cal_data_22    0x001E0F00
set dram_gen2_static_cal_data_23    0x001E0F00
set dram_gen2_static_cal_data_24    0x001E0F00
set dram_gen2_static_cal_data_25    0x001E0F00
set dram_gen2_static_cal_data_26    0x001E0F00
set dram_gen2_static_cal_data_27    0x001E0F00
set dram_gen2_static_cal_data_28    0x001E0F00
set dram_gen2_static_cal_data_29    0x001E0F00
set dram_gen2_static_cal_data_30    0x001E0F00
set dram_gen2_static_cal_data_31    0x001E0F00
set dram_gen2_static_cal_data_32    0x00000000
set dram_gen2_TX_CLK_PHS_DELAY      0x0
set dram_gen2_CLKM_DELAY            0xa
set dram_gen2_CLKM90_DELAY          0xf

set dram_gen2_size_auto_detection   disable
set dram_gen2_calibration_type      software ;#static or software

set dram_gen2_auto_calibration      disable

set dram_gen2_zq_calibration        disable
set dram_gen2_zq_impedance          50

set dram_gen2_drv_strength          normal

set dram_gen2_mrs_dll_enable        dll_enable  ;#dll_enable->0, dll_disable->1
set dram_gen2_mrs_drv_strength      reduced     ;#normal->0, reduced->1
set dram_gen2_mrs_odt               120         ;#(DDR1) none, (DDR2) 0, 50, 75, 150; (DDR3) 0, 20, 30, 40, 60, 120
set dram_gen2_mrs_additive_latency  0           ;#(DDR1) none, (DDR2) 0~5, DDR3 0~2

## globale information
set global_flash_type           spi_nor
#set global_preloader_image      "plr.img"
set global_mac_address          "11:22:33:44:55:66"
set global_plr_version          0x0009
set global_padding_unit         4K

##flash layout
set global_flash_layout [subst {
    { follow    bootloader1 "$global_bootloader_image" -reserve 64K}
    --runtime
    { follow    env 	    -reserve 64K }
    { follow    kernel1     -reserve 4M }
    { follow    rootfs1     -reserve 4M+512K }
}]

# the unit could be -in_byte, and -in_nor_erase

