/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <mmio.h>
#include <uart.h>
#include <gpio.h>
#include <ccmu.h>
#if DEBUG
#define thr rbr
#define dll rbr
#define dlh ier
#define iir fcr

static serial_hw_t *serial_ctrl_base = NULL;
static uint32_t uart_lock = 1;
#if 0
normal_gpio_set_t uart_ctrl[2] =
{
	{ 2, 8, 4, 1, 1, 0, {0}},//PB8: 4--RX
	{ 2, 9, 4, 1, 1, 0, {0}},//PB9: 4--TX
};
#endif

void sunxi_serial_init(int uart_port, void *gpio_cfg, int gpio_max)
{
	uint32_t reg, i;
	uint32_t uart_clk;

	if( (uart_port < 0) ||(uart_port > 0) )
	{
		return ;
	}
	//reset
	reg = mmio_read_32(CCMU_BUS_SOFT_RST_REG4);
	reg &= ~(1<<(CCM_UART_PORT_OFFSET + uart_port));
	mmio_write_32(CCMU_BUS_SOFT_RST_REG4,reg);
	for( i = 0; i < 100; i++ );
	reg |=  (1<<(CCM_UART_PORT_OFFSET + uart_port));
	mmio_write_32(CCMU_BUS_SOFT_RST_REG4,reg);
	//gate
	reg = mmio_read_32(CCMU_BUS_CLK_GATING_REG3);
	reg &= ~(1<<(CCM_UART_PORT_OFFSET + uart_port));
	mmio_write_32(CCMU_BUS_CLK_GATING_REG3,reg);
	for( i = 0; i < 100; i++ );
	reg |=  (1<<(CCM_UART_PORT_OFFSET + uart_port));
	mmio_write_32(CCMU_BUS_CLK_GATING_REG3,reg);

	//gpio
	//boot_set_gpio(gpio_cfg, gpio_max, 1);  //boot set,so not need to set again
	//uart init
	serial_ctrl_base = (serial_hw_t *)(SUNXI_UART0_BASE + uart_port * CCM_UART_ADDR_OFFSET);
	serial_ctrl_base->mcr = 0x3;
	uart_clk = (24000000 + 8 * UART_BAUD)/(16 * UART_BAUD);
	serial_ctrl_base->lcr |= 0x80;
	serial_ctrl_base->dlh = uart_clk>>8;
	serial_ctrl_base->dll = uart_clk&0xff;
	serial_ctrl_base->lcr &= ~0x80;
	serial_ctrl_base->lcr = ((PARITY&0x03)<<3) | ((STOP&0x01)<<2) | (DLEN&0x03);
	serial_ctrl_base->fcr = 0x7;
 
	uart_lock = 0;

	return;
}


void sunxi_serial_exit(void)
{
	uart_lock = 1;
}

void sunxi_serial_putc (char c)
{
	if (uart_lock)
		return;

	while((serial_ctrl_base->lsr & ( 1 << 6 )) == 0);
	serial_ctrl_base->thr = c;
}


char sunxi_serial_getc (void)
{
	if (uart_lock)
		return 0;

	while((serial_ctrl_base->lsr & 1) == 0);
	return serial_ctrl_base->rbr;

}

int sunxi_serial_tstc (void)
{
	return serial_ctrl_base->lsr & 1;
}
#endif /* DEBUG */

int console_init(unsigned long base_addr,
		unsigned int uart_clk, unsigned int baud_rate)
{
	sunxi_serial_init(0,NULL,0);
	return 0;
}

int console_exit()
{
	sunxi_serial_exit();
	return 0;
}

int console_putc(int c)
{
	sunxi_serial_putc(c);
	return 0;
}

int console_getc(void)
{
	return sunxi_serial_getc();
}

