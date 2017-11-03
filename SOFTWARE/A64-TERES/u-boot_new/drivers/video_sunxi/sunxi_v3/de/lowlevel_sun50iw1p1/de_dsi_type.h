#ifndef __DE_DSI_TYPE_H__
#define __DE_DSI_TYPE_H__

#include "de_lcd.h"

//
// Detail information of registers
//

typedef union
{
	u32 dwval;
	struct
	{
		u32 module_en               :  1 ;    // Default: 0x0;
		u32 res0                    : 15 ;    // Default: ;
		u32 version                 : 16 ;    // Default: 0x0102;
	} bits;
} DSI_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 phy_en                  :  1 ;    // Default: 0x0;
		u32 phy_rst                 :  1 ;    // Default: 0x0;
		u32 res0                    :  2 ;    // Default: ;
		u32 phy_lane_num            :  2 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
		u32 phy_clk_gating          :  1 ;    // Default: 0x0;
		u32 phy_clk_lane_enable     :  1 ;    // Default: 0x0;
		u32 res2                    : 22 ;    // Default: ;
	} bits;
} DSI_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 lp_clk_div              :  8 ;    // Default: 0x0;
		u32 res0                    : 24 ;    // Default: ;
	} bits;
} DSI_CTL2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 video_mode_en           :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 video_mode_cfg          :  2 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
		u32 vsa_lp_en               :  1 ;    // Default: 0x0;
		u32 vbp_lp_en               :  1 ;    // Default: 0x0;
		u32 vact_lp_en              :  1 ;    // Default: 0x0;
		u32 vfp_lp_en               :  1 ;    // Default: 0x0;
		u32 hbp_lp_en               :  1 ;    // Default: 0x0;
		u32 hfp_lp_en               :  1 ;    // Default: 0x0;
		u32 res2                    : 18 ;    // Default: ;
	} bits;
} DSI_VID_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 bta_per_frame           :  1 ;    // Default: 0x0;
		u32 lp_cmd_en               :  1 ;    // Default: 0x0;
		u32 res0                    :  2 ;    // Default: ;
		u32 pkt_multi_en            :  1 ;    // Default: 0x0;
		u32 pkt_null_in_hact        :  1 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
		u32 pkt_num_per_line        : 10 ;    // Default: 0x0;
		u32 res2                    :  2 ;    // Default: ;
		u32 pkt_null_size           : 10 ;    // Default: 0x0;
		u32 res3                    :  2 ;    // Default: ;
	} bits;
} DSI_VID_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 vsa                     :  4 ;    // Default: 0x0;
		u32 res0                    : 12 ;    // Default: ;
		u32 vbp                     :  6 ;    // Default: 0x0;
		u32 res1                    : 10 ;    // Default: ;
	} bits;
} DSI_VID_TIM0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 vact                    : 11 ;    // Default: 0x0;
		u32 res0                    :  5 ;    // Default: ;
		u32 vfp                     :  6 ;    // Default: 0x0;
		u32 res1                    : 10 ;    // Default: ;
	} bits;
} DSI_VID_TIM1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 pixels_per_pkg          : 11 ;    // Default: 0x0;
		u32 res0                    :  5 ;    // Default: ;
		u32 ht                      : 14 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
	} bits;
} DSI_VID_TIM2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 hsa                     :  9 ;    // Default: 0x0;
		u32 res0                    :  7 ;    // Default: ;
		u32 hbp                     :  9 ;    // Default: 0x0;
		u32 res1                    :  7 ;    // Default: ;
	} bits;
} DSI_VID_TIM3_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 res0                    :  8 ;    // Default: ;
		u32 dpi_format              :  3 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
		u32 video_mode_format_18bit :  1 ;    // Default: 0x0;
		u32 res2                    : 15 ;    // Default: ;
		u32 dpi_vid                 :  2 ;    // Default: 0x0;
		u32 res3                    :  2 ;    // Default: ;
	} bits;
} DSI_DPI_CFG0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 vsync_ploarity          :  1 ;    // Default: 0x0;
		u32 hsync_ploarity          :  1 ;    // Default: 0x0;
		u32 res0                    :  1 ;    // Default: ;
		u32 de_ploarity             :  1 ;    // Default: 0x0;
		u32 shutd_polarity          :  1 ;    // Default: 0x0;
		u32 colorm_polarity         :  1 ;    // Default: 0x0;
		u32 res1                    : 26 ;    // Default: ;
	} bits;
} DSI_DPI_CFG1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 dt                      :  6 ;    // Default: 0x0;
		u32 vc                      :  2 ;    // Default: 0x0;
		u32 wc                      : 16 ;    // Default: 0x0;
		u32 res0                    :  8 ;    // Default: ;
	} bits;
} DSI_PKG_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 pld_byte1               :  8 ;    // Default: 0x0;
		u32 pld_byte2               :  8 ;    // Default: 0x0;
		u32 pld_byte3               :  8 ;    // Default: 0x0;
		u32 pld_byte4               :  8 ;    // Default: 0x0;
	} bits;
} DSI_PKG_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 fifo_cmd_empty          :  1 ;    // Default: 0x1;
		u32 fifo_pld_w_empty        :  1 ;    // Default: 0x1;
		u32 fifo_pld_r_empty        :  1 ;    // Default: 0x1;
		u32 res0                    :  1 ;    // Default: ;
		u32 fifo_cmd_full           :  1 ;    // Default: 0x0;
		u32 fifo_pld_w_full         :  1 ;    // Default: 0x0;
		u32 fifo_pld_r_full         :  1 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
		u32 rd_cmd_busy             :  1 ;    // Default: 0x0;
		u32 res2                    : 23 ;    // Default: ;
	} bits;
} DSI_PKG_STATUS_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 bta_en                  :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 eotp_tx_en              :  1 ;    // Default: 0x0;
		u32 res1                    :  3 ;    // Default: ;
		u32 eotp_rx_en              :  1 ;    // Default: 0x0;
		u32 ecc_rx_en               :  1 ;    // Default: 0x0;
		u32 crc_rx_en               :  1 ;    // Default: 0x0;
		u32 res2                    : 17 ;    // Default: ;
		u32 vid_rx                  :  2 ;    // Default: 0x0;
		u32 res3                    :  2 ;    // Default: ;
	} bits;
} DSI_PKG_CTL2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 cmd_mode_en             :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 gen_sw_0p_tx_lp         :  1 ;    // Default: 0x0;
		u32 gen_sw_1p_tx_lp         :  1 ;    // Default: 0x0;
		u32 gen_sw_2p_tx_lp         :  1 ;    // Default: 0x0;
		u32 gen_lw_tx_lp            :  1 ;    // Default: 0x0;
		u32 gen_sr_0p_tx_lp         :  1 ;    // Default: 0x0;
		u32 gen_sr_1p_tx_lp         :  1 ;    // Default: 0x0;
		u32 gen_sr_2p_tx_lp         :  1 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
		u32 dcs_sw_0p_tx_lp         :  1 ;    // Default: 0x0;
		u32 dcs_sw_1p_tx_lp         :  1 ;    // Default: 0x0;
		u32 dcs_lw_tx_lp            :  1 ;    // Default: 0x0;
		u32 res2                    :  1 ;    // Default: ;
		u32 dcs_sr_0p_tx_lp         :  1 ;    // Default: 0x0;
		u32 res3                    :  3 ;    // Default: ;
		u32 max_rd_pkg_size_lp      :  1 ;    // Default: 0x0;
		u32 res4                    :  3 ;    // Default: ;
		u32 pkg_ack_req             :  1 ;    // Default: 0x0;
		u32 res5                    :  3 ;    // Default: ;
		u32 te_ack_en               :  1 ;    // Default: 0x0;
		u32 res6                    :  3 ;    // Default: ;
	} bits;
} DSI_CMD_CTL_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 res0                    :  4 ;    // Default: ;
		u32 dbi_in_format           :  4 ;    // Default: 0x0;
		u32 dbi_out_format          :  4 ;    // Default: 0x0;
		u32 lut_size_cfg            :  2 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
		u32 partion_mode            :  1 ;    // Default: 0x0;
		u32 res2                    : 11 ;    // Default: ;
		u32 dbi_vid                 :  2 ;    // Default: 0x0;
		u32 res3                    :  2 ;    // Default: ;
	} bits;
} DSI_DBI_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 wr_cmd_size             : 16 ;    // Default: 0x0;
		u32 allowed_cmd_size        : 16 ;    // Default: 0x0;
	} bits;
} DSI_DBI_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 dbi_fifo_cmd_empty      :  1 ;    // Default: 0x1;
		u32 dbi_fifo_pld_w_empty    :  1 ;    // Default: 0x1;
		u32 dbi_fifo_pld_r_empty    :  1 ;    // Default: 0x1;
		u32 res0                    :  1 ;    // Default: ;
		u32 dbi_fifo_cmd_full       :  1 ;    // Default: 0x0;
		u32 dbi_fifo_pld_w_full     :  1 ;    // Default: 0x0;
		u32 dbi_fifo_pld_r_full     :  1 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
		u32 dbi_rd_cmd_busy         :  1 ;    // Default: 0x0;
		u32 res2                    : 23 ;    // Default: ;
	} bits;
} DSI_DBI_CTL2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 to_clk_div              :  8 ;    // Default: 0x0;
		u32 res0                    : 24 ;    // Default: ;
	} bits;
} DSI_TO_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 to_hstx_set             : 16 ;    // Default: 0x0;
		u32 to_lprx_set             : 16 ;    // Default: 0x0;
	} bits;
} DSI_TO_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 lpcmd_time_invact       :  8 ;    // Default: 0x0;
		u32 res0                    :  8 ;    // Default: ;
		u32 lpcmd_time_outvact      :  8 ;    // Default: 0x0;
		u32 res1                    :  8 ;    // Default: ;
	} bits;
} DSI_VID_CTL2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 phy_lp2hs_set           :  8 ;    // Default: 0x0;
		u32 res0                    :  8 ;    // Default: ;
		u32 phy_hs2lp_set           :  8 ;    // Default: 0x0;
		u32 res1                    :  8 ;    // Default: ;
	} bits;
} DSI_PHY_CTL0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 phy_stop_set            :  8 ;    // Default: 0x0;
		u32 res0                    :  8 ;    // Default: ;
		u32 max_rd_set              : 15 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
	} bits;
} DSI_PHY_CTL1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 res0                    :  4 ;    // Default: ;
		u32 phy_ck_tx_ulps_req      :  1 ;    // Default: 0x0;
		u32 phy_ck_tx_ulps_exit     :  1 ;    // Default: 0x0;
		u32 res1                    :  2 ;    // Default: ;
		u32 phy_data_tx_ulps_req    :  1 ;    // Default: 0x0;
		u32 phy_data_tx_upls_exit   :  1 ;    // Default: 0x0;
		u32 res2                    :  6 ;    // Default: ;
		u32 phy_tx_triger           :  4 ;    // Default: 0x0;
		u32 res3                    : 12 ;    // Default: ;
	} bits;
} DSI_PHY_CTL2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 phy_lock                :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 phy_dir                 :  1 ;    // Default: 0x0;
		u32 res1                    :  3 ;    // Default: ;
		u32 phy_d0_stop             :  1 ;    // Default: 0x0;
		u32 phy_d1_stop             :  1 ;    // Default: 0x0;
		u32 phy_d2_stop             :  1 ;    // Default: 0x0;
		u32 phy_d3_stop             :  1 ;    // Default: 0x0;
		u32 phy_ck_stop             :  1 ;    // Default: 0x0;
		u32 res2                    :  3 ;    // Default: ;
		u32 phy_d0_no_ulps          :  1 ;    // Default: 0x1;
		u32 phy_d1_no_ulps          :  1 ;    // Default: 0x1;
		u32 phy_d2_no_ulps          :  1 ;    // Default: 0x1;
		u32 phy_d3_no_ulps          :  1 ;    // Default: 0x1;
		u32 phy_ck_no_ulps          :  1 ;    // Default: 0x1;
		u32 res3                    :  3 ;    // Default: ;
		u32 phy_d0rx_no_ulps        :  1 ;    // Default: 0x0;
		u32 res4                    :  7 ;    // Default: ;
	} bits;
} DSI_PHY_STATUS_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 phy_cfg_clr             :  1 ;    // Default: 0x1;
		u32 phy_cfg_clk             :  1 ;    // Default: 0x0;
		u32 res0                    :  2 ;    // Default: ;
		u32 phy_cfg_en              :  1 ;    // Default: 0x0;
		u32 res1                    : 11 ;    // Default: ;
		u32 phy_cfg_din             :  8 ;    // Default: 0x0;
		u32 phy_cfg_dout            :  8 ;    // Default: 0x0;
	} bits;
} DSI_PHY_CTL3_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 ack_with_err_0          :  1 ;    // Default: 0x0;
		u32 ack_with_err_1          :  1 ;    // Default: 0x0;
		u32 ack_with_err_2          :  1 ;    // Default: 0x0;
		u32 ack_with_err_3          :  1 ;    // Default: 0x0;
		u32 ack_with_err_4          :  1 ;    // Default: 0x0;
		u32 ack_with_err_5          :  1 ;    // Default: 0x0;
		u32 ack_with_err_6          :  1 ;    // Default: 0x0;
		u32 ack_with_err_7          :  1 ;    // Default: 0x0;
		u32 ack_with_err_8          :  1 ;    // Default: 0x0;
		u32 ack_with_err_9          :  1 ;    // Default: 0x0;
		u32 ack_with_err_10         :  1 ;    // Default: 0x0;
		u32 ack_with_err_11         :  1 ;    // Default: 0x0;
		u32 ack_with_err_12         :  1 ;    // Default: 0x0;
		u32 ack_with_err_13         :  1 ;    // Default: 0x0;
		u32 ack_with_err_14         :  1 ;    // Default: 0x0;
		u32 ack_with_err_15         :  1 ;    // Default: 0x0;
		u32 dphy_errors_0           :  1 ;    // Default: 0x0;
		u32 dphy_errors_1           :  1 ;    // Default: 0x0;
		u32 dphy_errors_2           :  1 ;    // Default: 0x0;
		u32 dphy_errors_3           :  1 ;    // Default: 0x0;
		u32 dphy_errors_4           :  1 ;    // Default: 0x0;
		u32 res0                    : 11 ;    // Default: ;
	} bits;
} DSI_IRQ_EN0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 ecc_single_err          :  1 ;    // Default: 0x0;
		u32 ecc_multi_err           :  1 ;    // Default: 0x0;
		u32 crc_err                 :  1 ;    // Default: 0x0;
		u32 eopt_err                :  1 ;    // Default: 0x0;
		u32 pkt_size_err            :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 dpi_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 res1                    :  3 ;    // Default: ;
		u32 gen_cmd_wr_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_send_err        :  1 ;    // Default: 0x0;
		u32 gen_pld_rd_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_recv_err        :  1 ;    // Default: 0x0;
		u32 res2                    :  3 ;    // Default: ;
		u32 dbi_cmd_wr_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_rd_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_recv_err        :  1 ;    // Default: 0x0;
		u32 dbi_illegal_comm_err    :  1 ;    // Default: 0x0;
		u32 res3                    :  3 ;    // Default: ;
		u32 to_hs_tx                :  1 ;    // Default: 0x0;
		u32 to_lp_rx                :  1 ;    // Default: 0x0;
		u32 res4                    :  2 ;    // Default: ;
	} bits;
} DSI_IRQ_EN1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 ack_with_err_0          :  1 ;    // Default: 0x0;
		u32 ack_with_err_1          :  1 ;    // Default: 0x0;
		u32 ack_with_err_2          :  1 ;    // Default: 0x0;
		u32 ack_with_err_3          :  1 ;    // Default: 0x0;
		u32 ack_with_err_4          :  1 ;    // Default: 0x0;
		u32 ack_with_err_5          :  1 ;    // Default: 0x0;
		u32 ack_with_err_6          :  1 ;    // Default: 0x0;
		u32 ack_with_err_7          :  1 ;    // Default: 0x0;
		u32 ack_with_err_8          :  1 ;    // Default: 0x0;
		u32 ack_with_err_9          :  1 ;    // Default: 0x0;
		u32 ack_with_err_10         :  1 ;    // Default: 0x0;
		u32 ack_with_err_11         :  1 ;    // Default: 0x0;
		u32 ack_with_err_12         :  1 ;    // Default: 0x0;
		u32 ack_with_err_13         :  1 ;    // Default: 0x0;
		u32 ack_with_err_14         :  1 ;    // Default: 0x0;
		u32 ack_with_err_15         :  1 ;    // Default: 0x0;
		u32 dphy_errors_0           :  1 ;    // Default: 0x0;
		u32 dphy_errors_1           :  1 ;    // Default: 0x0;
		u32 dphy_errors_2           :  1 ;    // Default: 0x0;
		u32 dphy_errors_3           :  1 ;    // Default: 0x0;
		u32 dphy_errors_4           :  1 ;    // Default: 0x0;
		u32 res0                    : 11 ;    // Default: ;
	} bits;
} DSI_IRQ_STATUS0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 ecc_single_err          :  1 ;    // Default: 0x0;
		u32 ecc_multi_err           :  1 ;    // Default: 0x0;
		u32 crc_err                 :  1 ;    // Default: 0x0;
		u32 eopt_err                :  1 ;    // Default: 0x0;
		u32 pkt_size_err            :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 dpi_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 res1                    :  3 ;    // Default: ;
		u32 gen_cmd_wr_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_send_err        :  1 ;    // Default: 0x0;
		u32 gen_pld_rd_err          :  1 ;    // Default: 0x0;
		u32 gen_pld_recv_err        :  1 ;    // Default: 0x0;
		u32 res2                    :  3 ;    // Default: ;
		u32 dbi_cmd_wr_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_wr_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_rd_err          :  1 ;    // Default: 0x0;
		u32 dbi_pld_recv_err        :  1 ;    // Default: 0x0;
		u32 dbi_illegal_comm_err    :  1 ;    // Default: 0x0;
		u32 res3                    :  3 ;    // Default: ;
		u32 to_hs_tx                :  1 ;    // Default: 0x0;
		u32 to_lp_rx                :  1 ;    // Default: 0x0;
		u32 res4                    :  2 ;    // Default: ;
	} bits;
} DSI_IRQ_STATUS1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 base_dir                :  4 ;    // Default: 0x0;
		u32 res0                    : 11 ;    // Default: ;
		u32 bist_on                 :  1 ;    // Default: 0x0;
		u32 turn_dis_0              :  1 ;    // Default: 0x0;
		u32 force_rx_0              :  1 ;    // Default: 0x0;
		u32 force_tx_stop_0         :  1 ;    // Default: 0x0;
		u32 res1                    :  1 ;    // Default: ;
		u32 turn_dis_1              :  1 ;    // Default: 0x0;
		u32 force_rx_1              :  1 ;    // Default: 0x0;
		u32 force_tx_stop_1         :  1 ;    // Default: 0x0;
		u32 res2                    :  1 ;    // Default: ;
		u32 turn_dis_2              :  1 ;    // Default: 0x0;
		u32 force_rx_2              :  1 ;    // Default: 0x0;
		u32 force_tx_stop_2         :  1 ;    // Default: 0x0;
		u32 res3                    :  1 ;    // Default: ;
		u32 turn_dis_3              :  1 ;    // Default: 0x0;
		u32 force_rx_3              :  1 ;    // Default: 0x0;
		u32 force_tx_stop_3         :  1 ;    // Default: 0x0;
		u32 res4                    :  1 ;    // Default: ;
	} bits;
} DSI_CFG0_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 dpi_color_mode          :  1 ;    // Default: 0x0;
		u32 dpi_shut_down           :  1 ;    // Default: 0x0;
		u32 dpi_src_format          :  2 ;    // Default: 0x0;
		u32 res0                    : 27 ;    // Default: ;
		u32 dpi_src                 :  1 ;    // Default: 0x0;
	} bits;
} DSI_CFG1_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 dbi_rst                 :  1 ;    // Default: 0x0;
		u32 lcd_te_en               :  1 ;    // Default: 0x0;
		u32 res0                    : 29 ;    // Default: ;
		u32 dbi_src                 :  1 ;    // Default: 0x0;
	} bits;
} DSI_CFG2_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 bist_en                 :  1 ;    // Default: 0x0;
		u32 res0                    :  3 ;    // Default: ;
		u32 sram0_bist_en           :  1 ;    // Default: 0x0;
		u32 sram1_bist_en           :  1 ;    // Default: 0x0;
		u32 sram2_bist_en           :  1 ;    // Default: 0x0;
		u32 sram3_bist_en           :  1 ;    // Default: 0x0;
		u32 res1                    : 12 ;    // Default: ;
		u32 reg_snk                 :  3 ;    // Default: 0x2;
		u32 res2                    :  5 ;    // Default: ;
		u32 reg_rint                :  2 ;    // Default: 0x2;
		u32 reg_rext                :  1 ;    // Default: 0x0;
		u32 res3                    :  1 ;    // Default: ;
	} bits;
} DSI_CFG3_t;

typedef union
{
	u32 dwval;
	struct
	{
		u32 bist_status             :  1 ;    // Default: 0x0;
		u32 res0                    : 31 ;    // Default: ;
	} bits;
} DSI_STATUS_t;

typedef union
{
	__u32 dwval;
	struct
	{
		__u32 res0                    ;    // Default: ;
	} bits;
}DSI_RESERVD_REG_t;

typedef struct
{
	DSI_CTL0_t			dsi_ctl0;			//0x000
	DSI_CTL1_t			dsi_ctl1;			//0x004
	DSI_CTL2_t			dsi_ctl2;           //0x008
	DSI_RESERVD_REG_t	dsi_reg00C;         //0x00C
	DSI_VID_CTL0_t		dsi_vid_ctl0;       //0x010
	DSI_RESERVD_REG_t	dsi_reg014[2];      //0x014~0x018
	DSI_VID_CTL1_t		dsi_vid_ctl1;       //0x01C
	DSI_VID_TIM0_t		dsi_vid_tim0;       //0x020
	DSI_VID_TIM1_t		dsi_vid_tim1;       //0x024
	DSI_VID_TIM2_t		dsi_vid_tim2;       //0x028
	DSI_VID_TIM3_t		dsi_vid_tim3;       //0x02C
	DSI_DPI_CFG0_t		dsi_dpi_cfg0;       //0x030
	DSI_RESERVD_REG_t	dsi_reg034[2];      //0x034~0x038
	DSI_DPI_CFG1_t		dsi_dpi_cfg1;       //0x03C
	DSI_PKG_CTL0_t		dsi_pkg_ctl0;       //0x040
	DSI_PKG_CTL1_t		dsi_pkg_ctl1;       //0x044
	DSI_PKG_STATUS_t	dsi_pkg_status;		//0x048
	DSI_RESERVD_REG_t	dsi_reg04C;         //0x04C
	DSI_PKG_CTL2_t		dsi_pkg_ctl2;       //0x050
	DSI_CMD_CTL_t		dsi_cmd_ctl; 		//0x054
	DSI_RESERVD_REG_t	dsi_reg058[10];     //0x058~0x07C
	DSI_DBI_CTL0_t		dsi_dbi_ctl0;       //0x080
	DSI_DBI_CTL1_t		dsi_dbi_ctl1;       //0x084
	DSI_DBI_CTL2_t		dsi_dbi_ctl2;       //0x088
	DSI_RESERVD_REG_t	dsi_reg08C[5];      //0x08C~0x09C
	DSI_TO_CTL0_t		dsi_to_ctl0;		//0x0A0
	DSI_TO_CTL1_t		dsi_to_ctl1;		//0x0A4
	DSI_VID_CTL2_t		dsi_vid_ctl2;       //0x0A8
	DSI_RESERVD_REG_t	dsi_reg0AC;         //0x0AC
	DSI_PHY_CTL0_t		dsi_phy_ctl0;       //0x0B0
	DSI_PHY_CTL1_t  	dsi_phy_ctl1;       //0x0B4
	DSI_RESERVD_REG_t	dsi_reg0B8[2];      //0x0B8~0x0BC
	DSI_PHY_CTL2_t		dsi_phy_ctl2;       //0x0C0
	DSI_PHY_STATUS_t	dsi_phy_status;     //0x0C4
	DSI_RESERVD_REG_t	dsi_reg0C8[10];     //0x0C8~0x0EC
	DSI_PHY_CTL3_t		dsi_phy_ctl3;       //0x0F0
	DSI_RESERVD_REG_t	dsi_reg0F4[59];     //0x0F4~0x1DC
	DSI_IRQ_EN0_t		dsi_irq_en0;        //0x1E0
	DSI_IRQ_EN1_t		dsi_irq_en1;        //0x1E4
	DSI_RESERVD_REG_t	dsi_reg1E8[2];      //0x1E8~0x1EC
	DSI_IRQ_STATUS0_t	dsi_irq_status0;    //0x1F0
	DSI_IRQ_STATUS1_t	dsi_irq_status1;    //0x1F4
	DSI_RESERVD_REG_t	dsi_reg1F8[130];    //0x1E8~0x1EC
	DSI_CFG0_t			dsi_cfg0;           //0x400
	DSI_CFG1_t			dsi_cfg1;           //0x404
	DSI_CFG2_t			dsi_cfg2;           //0x408
	DSI_CFG3_t			dsi_cfg3; 			//0x40C
	DSI_STATUS_t		dsi_statu_s;        //0x410
}__de_dsi_dev_t;

__u8 	dsi_ecc_pro(__u32 dsi_ph);
__u16 	dsi_crc_pro(__u8* pd_p,__u32 pd_bytes);
__u16 	dsi_crc_pro_pd_repeat(__u8 pd,__u32 pd_bytes);
//__s32 	dsi_dphy_init(__u32 sel,__panel_para_t * panel);
//__s32 	dsi_inst_init(__u32 sel,__panel_para_t * panel);
//__s32 	dsi_packet_init(__u32 sel,__panel_para_t * panel);



/* lvds */
typedef union
{
	u32 dwval;
	struct
	{
		u32 lvds_data_revert     :  4 ;    //default; 0:revert
		u32 lvds_clk_revert      :  1 ;    //default; 0: revert
		u32 res0                       : 15 ;    // default: ;
		u32 lvds_clk_sel         :  1 ;    // default: 0;
		u32 res1                       :  2 ;    // default: ;
		u32 lvds_correct_mode    :  1 ;    // default: 0;
		u32 lvds_debug_mode      :  1 ;    // default: 0;
		u32 lvds_debug_en        :  1 ;    // default: 0;
		u32 lvds_bitwidth        :  1 ;    // default: 0;
		u32 lvds_mode            :  1 ;    // default: 0;
		u32 lvds_dir             :  1 ;    // default: 0;
		u32 lvds_even_odd_dir    :  1 ;    // default: 0;
		u32 lvds_link	           :  1 ;    // default: 0;
		u32 lvds_en              :  1 ;    // default: 0;
	} bits;
} lvds_if_reg_t;

#endif
