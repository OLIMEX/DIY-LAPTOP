#ifndef _SUNXI_SCR_COMMON_H
#define _SUNXI_SCR_COMMON_H

extern  void scr_clear_ctl_reg(pscr_struct pscr);

extern  void scr_set_t_protocol(pscr_struct pscr, uint32_t config);

extern void scr_set_recv_parity(pscr_struct pscr, uint32_t config);

extern uint32_t scr_get_recv_parity(pscr_struct pscr);

extern  void scr_set_csr_config(pscr_struct pscr, uint32_t config);

extern  uint32_t scr_get_csr_config(pscr_struct pscr);

extern  uint32_t scr_detected(pscr_struct pscr);

extern  void scr_start_deactivation(pscr_struct pscr);

extern  void scr_start_activation(pscr_struct pscr);

extern  void scr_start_warmreset(pscr_struct pscr);

extern  void scr_stop_clock(pscr_struct pscr);

extern  void scr_restart_clock(pscr_struct pscr);

extern  void scr_global_interrupt_enable(pscr_struct pscr);

extern  void scr_global_interrupt_disable(pscr_struct pscr);

extern  void scr_receive_enable(pscr_struct pscr);

extern  void scr_receive_disable(pscr_struct pscr);

extern  void scr_transmit_enable(pscr_struct pscr);

extern  void scr_transmit_disable(pscr_struct pscr);

extern  void scr_set_interrupt_enable(pscr_struct pscr, uint32_t bm);

extern  void scr_set_interrupt_disable(pscr_struct pscr, uint32_t bm);

extern  uint32_t scr_get_interrupt_enable(pscr_struct pscr);

extern  uint32_t scr_get_interrupt_status(pscr_struct pscr);

extern  void scr_clear_interrupt_status(pscr_struct pscr, uint32_t bm);

extern  void scr_flush_txfifo(pscr_struct pscr);

extern  void scr_flush_rxfifo(pscr_struct pscr);

extern  uint32_t scr_txfifo_is_empty(pscr_struct pscr);

extern  uint32_t scr_txfifo_is_full(pscr_struct pscr);

extern  uint32_t scr_rxfifo_is_empty(pscr_struct pscr);

extern  uint32_t scr_rxfifo_is_full(pscr_struct pscr);

extern  void scr_set_txfifo_threshold(pscr_struct pscr, uint32_t thh);

extern  void scr_set_rxfifo_threshold(pscr_struct pscr, uint32_t thh);

extern  uint32_t scr_get_txfifo_count(pscr_struct pscr);

extern  uint32_t scr_get_rxfifo_count(pscr_struct pscr);

extern  void scr_set_tx_repeat(pscr_struct pscr, uint32_t repeat);

extern  void scr_set_rx_repeat(pscr_struct pscr, uint32_t repeat);

extern  void scr_set_scclk_divisor(pscr_struct pscr, uint32_t divisor);

extern  void scr_set_baud_divisor(pscr_struct pscr, uint32_t divisor);

extern  uint32_t scr_get_scclk_divisor(pscr_struct pscr);

extern  uint32_t scr_get_baud_divisor(pscr_struct pscr);

extern  void scr_set_activation_time(pscr_struct pscr, uint32_t scclk);

extern  void scr_set_reset_time(pscr_struct pscr, uint32_t scclk);

extern  void scr_set_atrlimit_time(pscr_struct pscr, uint32_t scclk);

extern  uint32_t scr_get_line_time(pscr_struct pscr);

extern  void scr_set_guard_time(pscr_struct pscr, uint32_t etu);

extern  void scr_set_chlimit_time(pscr_struct pscr, uint32_t etu);

extern  uint32_t scr_get_character_time(pscr_struct pscr);

extern  void scr_auto_vpp_enable(pscr_struct pscr);

extern  void scr_auto_vpp_disable(pscr_struct pscr);

extern  void scr_write_fifo(pscr_struct pscr, uint8_t data);

extern  uint8_t scr_read_fifo(pscr_struct pscr);

extern  uint32_t scr_get_fsm(pscr_struct pscr);

#endif
