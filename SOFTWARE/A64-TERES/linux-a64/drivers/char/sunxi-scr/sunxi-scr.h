#ifndef _SUNXI_SCR_H
#define _SUNXI_SCR_H

#include <linux/types.h>
/* only complie pass */

#define SCR_MODULE_NAME "smartcard"

#define SCR_FIFO_DEPTH                  16
#define SCR_CSR_OFF                     (0x000)
#define SCR_INTEN_OFF                   (0x004)
#define SCR_INTST_OFF                   (0x008)
#define SCR_FCSR_OFF                    (0x00c)
#define SCR_FCNT_OFF                    (0x010)
#define SCR_RPT_OFF                     (0x014)
#define SCR_DIV_OFF                     (0x018)
#define SCR_LTIM_OFF                    (0x01c)
#define SCR_CTIM_OFF                    (0x020)
#define SCR_LCTL_OFF                    (0x030)
#define SCR_FSM_OFF                     (0x03c)
#define SCR_FIFO_OFF                    (0x100)

#define SCR_INTSTA_DEACT                (0x1<<23)
#define SCR_INTSTA_ACT                  (0x1<<22)
#define SCR_INTSTA_INS                  (0x1<<21)
#define SCR_INTSTA_REM                  (0x1<<20)
#define SCR_INTSTA_ATRDONE              (0x1<<19)
#define SCR_INTSTA_ATRFAIL              (0x1<<18)
#define SCR_INTSTA_CHTO                 (0x1<<17)  //Character Timout
#define SCR_INTSTA_CLOCK                (0x1<<16)
#define SCR_INTSTA_RXPERR               (0x1<<12)
#define SCR_INTSTA_RXDONE               (0x1<<11)
#define SCR_INTSTA_RXFTH                (0x1<<10)
#define SCR_INTSTA_RXFFULL              (0x1<<9)
#define SCR_INTSTA_TXPERR               (0x1<<4)
#define SCR_INTSTA_TXDONE               (0x1<<3)
#define SCR_INTSTA_TXFTH                (0x1<<2)
#define SCR_INTSTA_TXFEMPTY             (0x1<<1)
#define SCR_INTSTA_TXFDONE              (0x1<<0)

#define SCR_BUFFER_SIZE_MASK            0xff//256	//0x3f  //64
#define MAX_ATR_LEN				33

#define SCR_ATR_RESP_INVALID            0
#define SCR_ATR_RESP_FAIL               1
#define SCR_ATR_RESP_OK                 2

typedef enum
{
	SCR_CARD_OUT,
	SCR_CARD_IN,
}scr_status_t;

typedef struct
{
	volatile uint32_t wptr;
	volatile uint32_t rptr;
	uint8_t buffer[SCR_BUFFER_SIZE_MASK+1];
}scr_buffer, *pscr_buffer;

typedef struct
{
	uint32_t atr_len;
	uint8_t buffer[MAX_ATR_LEN];
}atr_buffer;

typedef struct {
	uint16_t f;
	uint16_t d;
	uint16_t freq;
	uint16_t recv_no_parity;
} cardPara;

typedef struct {
	void __iomem * reg_base;
	struct clk *scr_clk;
	struct clk *scr_clk_source;
	struct platform_device *scr_device;
	uint32_t clk_freq;
	uint32_t irq_no;
	volatile uint32_t irq_accsta;
	volatile uint32_t irq_cursta;
	//control and status register config
	uint32_t csr_config;
	//interrupt enable bit map
	uint32_t inten_bm;
	//txfifo threshold
	uint32_t txfifo_thh;
	//rxfifo threahold
	uint32_t rxfifo_thh;
	//tx repeat
	uint32_t tx_repeat;
	//rx repeat
	uint32_t rx_repeat;
	//scclk divisor
	uint32_t scclk_div;
	//baud divisor
	uint32_t baud_div;
	//activation/deactivation time, in scclk cycles
	uint32_t act_time;
	//reset time, in scclk cycles
	uint32_t rst_time;
	//ATR limit time, in scclk cycles
	uint32_t atr_time;
	//gaurd time, in ETUs
	uint32_t guard_time;
	//character limit time, in ETUs
	uint32_t chlimit_time;

	scr_buffer rxbuf;
	scr_buffer txbuf;

	volatile uint32_t detected;
	volatile uint32_t activated;
	volatile uint32_t atr_resp;

	uint32_t chto_flag;
}scr_struct, *pscr_struct;

#define	SCR_IOC_MAGIC		'c'

#define	SCR_IOCGSTATUS		_IOR (SCR_IOC_MAGIC, 0, uint32_t *)
#define	SCR_IOCRESET		_IOWR(SCR_IOC_MAGIC, 1, atr_buffer *)
#define	SCR_IOCGPARA		_IOR(SCR_IOC_MAGIC, 2, cardPara *)
#define	SCR_IOCSPARA		_IOW(SCR_IOC_MAGIC, 3, cardPara *)

enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_INT = 1U << 1,
	DEBUG_DATA_INFO = 1U << 2,
	DEBUG_SUSPEND = 1U << 3,
};

#define dprintk(level_mask, fmt, arg...)	if (unlikely(scr_debug_mask & level_mask)) \
	 printk(KERN_DEBUG fmt , ## arg)

#endif

