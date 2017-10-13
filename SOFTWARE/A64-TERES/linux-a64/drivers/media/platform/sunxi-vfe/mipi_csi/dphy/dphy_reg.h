
#ifndef __DPHY_REG_H__
#define __DPHY_REG_H__

#include "dphy.h"
#define MAX_MIPI_DPHY 1

extern int  dphy_reg_map(unsigned int sel, unsigned long addr_base);
extern void dphy_ana_init(unsigned int sel);
extern void dphy_ana_exit(unsigned int sel);
extern void dphy_enable(unsigned int sel);
extern void dphy_disable(unsigned int sel);
extern void dphy_set_data_lane(unsigned int sel, unsigned char lane_num);                       
extern void dphy_rx_enable(unsigned int sel, unsigned char lane_num);
extern void dphy_rx_disable(unsigned int sel, unsigned char lane_num);
extern void dphy_rx_dbc_enable(unsigned int sel);
extern void dphy_rx_dbc_disable(unsigned int sel);
extern void dphy_rx_hs_clk_miss_cnt_enable(unsigned int sel);
extern void dphy_rx_hs_clk_miss_cnt_disable(unsigned int sel);
extern void dphy_rx_hs_sync_cnt_enable(unsigned int sel);
extern void dphy_rx_hs_sync_cnt_disable(unsigned int sel);
extern void dphy_rx_lp_to_cnt_enable(unsigned int sel);
extern void dphy_rx_lp_to_cnt_disable(unsigned int sel);
extern void dphy_rx_freq_cnt_enable(unsigned int sel);
extern void dphy_rx_freq_cnt_disable(unsigned int sel);
extern void dphy_rx_set_hs_clk_miss(unsigned int sel, unsigned char cnt);
extern void dphy_rx_set_hs_sync_to(unsigned int sel, unsigned char cnt);
extern void dphy_rx_set_lp_to(unsigned int sel, unsigned char cnt);
extern void dphy_rx_set_rx_dly(unsigned int sel, unsigned short cnt);
extern void dphy_rx_set_lprst_dly(unsigned int sel, unsigned char cnt);
extern void dphy_rx_set_lp_ulps_wp(unsigned int sel, unsigned int cnt);
extern void dphy_rx_set_entm_to_enrx_dly(unsigned int sel, unsigned char cnt);
extern void dphy_int_enable(unsigned int sel, enum dphy_int dphy_int);
extern void dphy_int_disable(unsigned int sel, enum dphy_int dphy_int);
extern int  dphy_get_int_status(unsigned int sel, enum dphy_int dphy_int);
extern void dphy_clear_int_status(unsigned int sel, enum dphy_int dphy_int);

extern unsigned char  dphy_get_data_lane(unsigned int sel); 
extern unsigned char  dphy_rx_get_hs_clk_miss(unsigned int sel);
extern unsigned char  dphy_rx_get_hs_sync_to(unsigned int sel);
extern unsigned char  dphy_rx_get_lp_to(unsigned int sel);
extern unsigned short dphy_rx_get_rx_dly(unsigned int sel);
extern unsigned char  dphy_rx_get_lprst_dly(unsigned int sel);
extern unsigned int   dphy_rx_get_lp_ulps_wp(unsigned int sel);
extern unsigned char  dphy_rx_get_entm_to_enrx_dly(unsigned int sel);
extern unsigned short dphy_rx_get_freq_cnt(unsigned int sel);
extern unsigned char  dphy_get_hs_data(unsigned int sel, enum dphy_lane lane);
extern enum dphy_lane_state dphy_get_lane_state(unsigned int sel, enum dphy_lane lane);
                                             
#endif
