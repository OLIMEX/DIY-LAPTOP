#include <asm/io.h>
#include "sunxi-scr.h"

void scr_clear_ctl_reg(pscr_struct pscr)
{
	writel(0x0, pscr->reg_base + SCR_CSR_OFF);
}

void scr_set_t_protocol(pscr_struct pscr, uint32_t config)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x3<<22);
	reg_val |= (config << 22);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_set_recv_parity(pscr_struct pscr, uint32_t config)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (config << 18);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

uint32_t scr_get_recv_parity(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val =  readl(pscr->reg_base + SCR_CSR_OFF);

	return (reg_val & (0x1<<18));
}

void scr_set_csr_config(pscr_struct pscr, uint32_t config)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x1ff<<16);
	reg_val |= config & (0x1ff<<16);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

uint32_t scr_get_csr_config(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_CSR_OFF);
}

uint32_t scr_detected(pscr_struct pscr)
{
	return (readl(pscr->reg_base + SCR_CSR_OFF)>>31);
}

void scr_start_deactivation(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<11);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_start_activation(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<10);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_start_warmreset(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<9);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_stop_clock(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<8);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_restart_clock(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x1<<8);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_global_interrupt_enable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<2);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_global_interrupt_disable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x1<<2);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_receive_enable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<1);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_receive_disable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x1<<1);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_transmit_enable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val |= (0x1<<0);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_transmit_disable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CSR_OFF);
	reg_val &= ~(0x1<<0);
	writel(reg_val, pscr->reg_base + SCR_CSR_OFF);
}

void scr_set_interrupt_enable(pscr_struct pscr, uint32_t bm)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_INTEN_OFF);
	reg_val |= bm;
	writel(reg_val, pscr->reg_base + SCR_INTEN_OFF);
}

void scr_set_interrupt_disable(pscr_struct pscr, uint32_t bm)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_INTEN_OFF);
	reg_val &= ~bm;
	writel(reg_val, pscr->reg_base + SCR_INTEN_OFF);
}

uint32_t scr_get_interrupt_enable(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_INTEN_OFF);
}

uint32_t scr_get_interrupt_status(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_INTST_OFF);
}

void scr_clear_interrupt_status(pscr_struct pscr, uint32_t bm)
{
	writel(bm, pscr->reg_base + SCR_INTST_OFF);
}

void scr_flush_txfifo(pscr_struct pscr)
{
	writel((0x1<<2), pscr->reg_base + SCR_FCSR_OFF);
}

void scr_flush_rxfifo(pscr_struct pscr)
{
	writel((0x1<<10), pscr->reg_base + SCR_FCSR_OFF);
}

uint32_t scr_txfifo_is_empty(pscr_struct pscr)
{
	return (readl(pscr->reg_base + SCR_FCSR_OFF)&0x1);
}

uint32_t scr_txfifo_is_full(pscr_struct pscr)
{
	return ((readl(pscr->reg_base + SCR_FCSR_OFF)>>1)&0x1);
}

uint32_t scr_rxfifo_is_empty(pscr_struct pscr)
{
	return ((readl(pscr->reg_base + SCR_FCSR_OFF)>>8)&0x1);
}

uint32_t scr_rxfifo_is_full(pscr_struct pscr)
{
	return ((readl(pscr->reg_base + SCR_FCSR_OFF)>>9)&0x1);
}

void scr_set_txfifo_threshold(pscr_struct pscr, uint32_t thh)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_FCNT_OFF);
	reg_val &= ~(0xff<<16);
	reg_val |= (thh&0xff)<<16;
	writel(reg_val, pscr->reg_base + SCR_FCNT_OFF);
}

void scr_set_rxfifo_threshold(pscr_struct pscr, uint32_t thh)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_FCNT_OFF);
	reg_val &= ~(0xffU<<24);
	reg_val |= (thh&0xff)<<24;
	writel(reg_val, pscr->reg_base + SCR_FCNT_OFF);
}

uint32_t scr_get_txfifo_count(pscr_struct pscr)
{
	return (readl(pscr->reg_base + SCR_FCNT_OFF)&0xff);
}

uint32_t scr_get_rxfifo_count(pscr_struct pscr)
{
	return ((readl(pscr->reg_base + SCR_FCNT_OFF)>>8)&0xff);
}

void scr_set_tx_repeat(pscr_struct pscr, uint32_t repeat)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_RPT_OFF);
	reg_val &= ~(0xf<<0);
	reg_val |= (repeat&0xf)<<0;
	writel(reg_val, pscr->reg_base + SCR_RPT_OFF);
}

void scr_set_rx_repeat(pscr_struct pscr, uint32_t repeat)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_RPT_OFF);
	reg_val &= ~(0xf<<4);
	reg_val |= (repeat&0xf)<<4;
	writel(reg_val, pscr->reg_base + SCR_RPT_OFF);
}

void scr_set_scclk_divisor(pscr_struct pscr, uint32_t divisor)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_DIV_OFF);
	reg_val &= ~(0xffff<<0);
	reg_val |= (divisor&0xffff)<<0;
	writel(reg_val, pscr->reg_base + SCR_DIV_OFF);
}

void scr_set_baud_divisor(pscr_struct pscr, uint32_t divisor)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_DIV_OFF);
	reg_val &= ~(0xffffU<<16);
	reg_val |= (divisor&0xffff)<<16;
	writel(reg_val, pscr->reg_base + SCR_DIV_OFF);
}

uint32_t scr_get_scclk_divisor(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_DIV_OFF)&0xffff;
}

uint32_t scr_get_baud_divisor(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_DIV_OFF)>>16;
}

void scr_set_activation_time(pscr_struct pscr, uint32_t scclk)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_LTIM_OFF);
	reg_val &= ~(0xff<<0);
	reg_val |= (scclk&0xff)<<0;
	writel(reg_val, pscr->reg_base + SCR_LTIM_OFF);
}

void scr_set_reset_time(pscr_struct pscr, uint32_t scclk)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_LTIM_OFF);
	reg_val &= ~(0xff<<8);
	reg_val |= (scclk&0xff)<<8;
	writel(reg_val, pscr->reg_base + SCR_LTIM_OFF);
}

void scr_set_atrlimit_time(pscr_struct pscr, uint32_t scclk)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_LTIM_OFF);
	reg_val &= ~(0xff<<16);
	reg_val |= (scclk&0xff)<<16;
	writel(reg_val, pscr->reg_base + SCR_LTIM_OFF);
}

uint32_t scr_get_line_time(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_LTIM_OFF);
}

void scr_set_guard_time(pscr_struct pscr, uint32_t etu)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CTIM_OFF);
	reg_val &= ~(0xff<<0);
	reg_val |= (etu&0xff)<<0;
	writel(reg_val, pscr->reg_base + SCR_CTIM_OFF);
}

void scr_set_chlimit_time(pscr_struct pscr, uint32_t etu)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_CTIM_OFF);
	reg_val &= ~(0xffffU<<16);
	reg_val |= (etu&0xffff)<<16;
	writel(reg_val, pscr->reg_base + SCR_CTIM_OFF);
}

uint32_t scr_get_character_time(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_CTIM_OFF);
}

void scr_auto_vpp_enable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_LCTL_OFF);
	reg_val |= (0x1<<5);
	writel(reg_val, pscr->reg_base + SCR_LCTL_OFF);
}

void scr_auto_vpp_disable(pscr_struct pscr)
{
	uint32_t reg_val;

	reg_val = readl(pscr->reg_base + SCR_LCTL_OFF);
	reg_val &= (~(0x1<<5));
	writel(reg_val, pscr->reg_base + SCR_LCTL_OFF);
}

void scr_write_fifo(pscr_struct pscr, uint8_t data)
{
	writel((uint32_t)data, pscr->reg_base + SCR_FIFO_OFF);
}

uint8_t scr_read_fifo(pscr_struct pscr)
{
	return ((uint8_t)readl(pscr->reg_base + SCR_FIFO_OFF));
}

uint32_t scr_get_fsm(pscr_struct pscr)
{
	return readl(pscr->reg_base + SCR_FSM_OFF);
}

