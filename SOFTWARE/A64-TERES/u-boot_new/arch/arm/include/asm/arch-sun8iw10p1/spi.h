/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef   __SPI_H__
#define   __SPI_H__

#include  <asm/arch/cpu.h>

#define RET_OK		(0)
#define RET_FAIL	(-1)

#define RET_TRUE	(0)
#define RET_FALSE	(-1)

#define DMA_TYPE_SPI0           (23)      /* port 23 */
#define DMA_TYPE_SPI1           (24)      /* port 24 */
#define DMA_CFG_DST_DRQ_SPI0 		(DMA_TYPE_SPI0      << 16)
#define DMA_CFG_DST_DRQ_SPI1 		(DMA_TYPE_SPI1      << 16)
#define DMA_CFG_SRC_DRQ_SPI0 		(DMA_TYPE_SPI0      << 0)
#define DMA_CFG_SRC_DRQ_SPI1 		(DMA_TYPE_SPI1      << 0)
#define SPI_TRANS_MODE 0
#define SPI_CLK_SRC		(1)	//0-24M, 1-PLL6
#define SPI_MCLK		(40000000)
#define MAX_SPI_NUM 2

#define set_wbit(addr, v)   (*((volatile unsigned long  *)(addr)) |=  (unsigned long)(v))
#define clr_wbit(addr, v)   (*((volatile unsigned long  *)(addr)) &= ~(unsigned long)(v))
//#define writeb(value,addr)  (*((volatile __u8 *)(addr))  = (value))   /* byte output */

#define DMA_CFG_SRC_BST1_WIDTH8     ((DMA_CFG_BST1 << 7) | (DMA_CFG_WID8 << 9))
#define DMA_CFG_DST_BST1_WIDTH8     ((DMA_CFG_BST1 << 23) | (DMA_CFG_WID8 << 25))
#define DRAM_MEM_BASE		(0x40000000)
#define DMA_IRQ_PEND0_REG   DMA_IRQ_PEND_REG



/* run time control */
#define TEST_SPI_NO		(0)
#define SPI_DEFAULT_CLK	(40000000)
#define SPI_TX_WL		(0x20)
#define SPI_RX_WL		(0x20)
#define SPI_FIFO_SIZE	(64)
#define SPI_CLK_SRC		(1)	//0-24M, 1-PLL6
#define SPI_MCLK		(40000000)

#define SPIC_BASE_OS	(0x1000)
#define SPI_BASE        (0x01c68000)


//#define SPI_BASE		(SPI0_BASE + SPIC_BASE_OS * (spi_no))
//#define SPI_IRQNO(_n)	(GIC_SRC_SPI0 + (_n))

#define SPI_VAR		(SPI_BASE + 0x00)
#define SPI_GCR		(SPI_BASE + 0x04)
#define SPI_TCR		(SPI_BASE + 0x08)
#define SPI_IER		(SPI_BASE + 0x10)
#define SPI_ISR		(SPI_BASE + 0x14)
#define SPI_FCR		(SPI_BASE + 0x18)
#define SPI_FSR		(SPI_BASE + 0x1c)
#define SPI_WCR		(SPI_BASE + 0x20)
#define SPI_CCR		(SPI_BASE + 0x24)
#define SPI_MBC		(SPI_BASE + 0x30)
#define SPI_MTC		(SPI_BASE + 0x34)
#define SPI_BCC		(SPI_BASE + 0x38)
#define SPI_TXD		(SPI_BASE + 0x200)
#define SPI_RXD		(SPI_BASE + 0x300)

/* bit field of registers */
#define SPI_SOFT_RST	(1U << 31)
#define SPI_TXPAUSE_EN	(1U << 7)
#define SPI_MASTER		(1U << 1)
#define SPI_ENABLE		(1U << 0)

#define SPI_EXCHANGE	(1U << 31)
#define SPI_SAMPLE_MODE	(1U << 13)
#define SPI_LSB_MODE	(1U << 12)
#define SPI_SAMPLE_CTRL	(1U << 11)
#define SPI_RAPIDS_MODE	(1U << 10)
#define SPI_DUMMY_1		(1U << 9)
#define SPI_DHB			(1U << 8)
#define SPI_SET_SS_1	(1U << 7)
#define SPI_SS_MANUAL	(1U << 6)
#define SPI_SEL_SS0		(0U << 4)
#define SPI_SEL_SS1		(1U << 4)
#define SPI_SEL_SS2		(2U << 4)
#define SPI_SEL_SS3		(3U << 4)
#define SPI_SS_N_INBST	(1U << 3)
#define SPI_SS_ACTIVE0	(1U << 2)
#define SPI_MODE0		(0U << 0)
#define SPI_MODE1		(1U << 0)
#define SPI_MODE2		(2U << 0)
#define SPI_MODE3		(3U << 0)

//#define SPI_CPHA        (1U << 0)

#define SPI_SS_INT		(1U << 13)
#define SPI_TC_INT		(1U << 12)
#define SPI_TXUR_INT	(1U << 11)
#define SPI_TXOF_INT	(1U << 10)
#define SPI_RXUR_INT	(1U << 9)
#define SPI_RXOF_INT	(1U << 8)
#define SPI_TXFULL_INT	(1U << 6)
#define SPI_TXEMPT_INT	(1U << 5)
#define SPI_TXREQ_INT	(1U << 4)
#define SPI_RXFULL_INT	(1U << 2)
#define SPI_RXEMPT_INT	(1U << 1)
#define SPI_RXREQ_INT	(1U << 0)
#define SPI_ERROR_INT	(SPI_TXUR_INT|SPI_TXOF_INT|SPI_RXUR_INT|SPI_RXOF_INT)

#define SPI_TXFIFO_RST	(1U << 31)
#define SPI_TXFIFO_TST	(1U << 30)
#define SPI_TXDMAREQ_EN	(1U << 24)
#define SPI_RXFIFO_RST	(1U << 15)
#define SPI_RXFIFO_TST	(1U << 14)
#define SPI_RXDMAREQ_EN	(1U << 8)
#define SPI_MASTER_DUAL	(1U << 28)




extern int   spic_init(unsigned int spi_no);
extern int   spic_exit(unsigned int spi_no);
//extern int   spic_rw  (unsigned int tcnt, void* txbuf, unsigned int rcnt, void* rxbuf);
extern int spic_rw( u32 txlen, void* txbuff, u32 rxlen, void* rxbuff);



#endif

