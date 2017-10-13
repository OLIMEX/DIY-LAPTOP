/*
 * drivers/i2c/busses/i2c-sunxi.c
 *
 * Copyright (C) 2013 Allwinner.
 * Pan Nan <pannan@reuuimllatech.com>
 *
 * SUNXI TWI Controller Driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * 2013.5.3 Mintow <duanmintao@allwinnertech.com>
 *    Adapt to all the new chip of Allwinner. Support sun8i/sun9i.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/kernel_ut.h>
#include <linux/pinctrl/consumer.h>
#include <linux/clk/sunxi.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_i2c.h>

#include "i2c-sunxi.h"

/* For debug */
#define I2C_EXIT()  		pr_debug("%s()%d - %s \n", __func__, __LINE__, "Exit")
#define I2C_ENTER() 		pr_debug("%s()%d - %s \n", __func__, __LINE__, "Enter ...")
#define I2C_DBG(fmt, arg...)	pr_debug("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define I2C_ERR(fmt, arg...)	pr_warn("%s()%d - "fmt, __func__, __LINE__, ##arg)

#ifndef CONFIG_SUNXI_I2C_PRINT_TRANSFER_INFO
static int bus_transfer_dbg = -1;
module_param_named(transfer_debug, bus_transfer_dbg, int, S_IRUGO | S_IWUSR | S_IWGRP);
#endif

UT_DECLARE();

#define SUNXI_I2C_OK      0
#define SUNXI_I2C_FAIL   -1
#define SUNXI_I2C_RETRY  -2
#define SUNXI_I2C_SFAIL  -3  /* start fail */
#define SUNXI_I2C_TFAIL  -4  /* stop  fail */

static int twi_used_mask = 0;

/* I2C transfer status */
enum
{
	I2C_XFER_IDLE    = 0x1,
	I2C_XFER_START   = 0x2,
	I2C_XFER_RUNNING = 0x4,
};

struct sunxi_i2c {
	int 				bus_num;
	unsigned int      	status; /* start, running, idle */
	unsigned int      	suspended:1;
	struct i2c_adapter	adap;

	spinlock_t          lock; /* syn */
	wait_queue_head_t   wait;

	struct i2c_msg      *msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;

#ifdef CONFIG_EVB_PLATFORM
	struct clk         *mclk;
#endif

	unsigned int     	bus_freq;
	int					irq;
	unsigned int 		debug_state; /* log the twi machine state */

	void __iomem	 	*base_addr;

	struct pinctrl		*pctrl;
};

/* clear the interrupt flag */
static inline void twi_clear_irq_flag(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	/* start and stop bit should be 0 */
	reg_val |= TWI_CTL_INTFLG;
	reg_val &= ~(TWI_CTL_STA | TWI_CTL_STP);
	writel(reg_val ,base_addr + TWI_CTL_REG);
	/* read two more times to make sure that interrupt flag does really be cleared */
	{
		unsigned int temp;
		temp = readl(base_addr + TWI_CTL_REG);
		temp |= readl(base_addr + TWI_CTL_REG);
	}
}

/* get data first, then clear flag */
static inline void twi_get_byte(void __iomem *base_addr, unsigned char  *buffer)
{
	*buffer = (unsigned char)( TWI_DATA_MASK & readl(base_addr + TWI_DATA_REG) );
	twi_clear_irq_flag(base_addr);
}

/* only get data, we will clear the flag when stop */
static inline void twi_get_last_byte(void __iomem *base_addr, unsigned char  *buffer)
{
	*buffer = (unsigned char)( TWI_DATA_MASK & readl(base_addr + TWI_DATA_REG) );
}

/* write data and clear irq flag to trigger send flow */
static inline void twi_put_byte(void __iomem *base_addr, const unsigned char *buffer)
{
	writel((unsigned int)*buffer, base_addr + TWI_DATA_REG);
	twi_clear_irq_flag(base_addr);
}

static inline void twi_enable_irq(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);

	/*
	 * 1 when enable irq for next operation, set intflag to 0 to prevent to clear it by a mistake
	 *   (intflag bit is write-1-to-clear bit)
	 * 2 Similarly, mask START bit and STOP bit to prevent to set it twice by a mistake
	 *   (START bit and STOP bit are self-clear-to-0 bits)
	 */
	reg_val |= TWI_CTL_INTEN;
	reg_val &= ~(TWI_CTL_STA | TWI_CTL_STP | TWI_CTL_INTFLG);
	writel(reg_val, base_addr + TWI_CTL_REG);
}

static inline void twi_disable_irq(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val &= ~TWI_CTL_INTEN;
	reg_val &= ~(TWI_CTL_STA | TWI_CTL_STP | TWI_CTL_INTFLG);
	writel(reg_val, base_addr + TWI_CTL_REG);
}

static inline void twi_disable_bus(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val &= ~TWI_CTL_BUSEN;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

static inline void twi_enable_bus(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val |= TWI_CTL_BUSEN;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

/* trigger start signal, the start bit will be cleared automatically */
static inline void twi_set_start(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val |= TWI_CTL_STA;
	reg_val &= ~TWI_CTL_INTFLG;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

/* get start bit status, poll if start signal is sent */
static inline unsigned int twi_get_start(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val >>= 5;
	return reg_val & 1;
}

/* trigger stop signal, the stop bit will be cleared automatically */
static inline void twi_set_stop(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val |= TWI_CTL_STP;
	reg_val &= ~TWI_CTL_INTFLG;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

/* get stop bit status, poll if stop signal is sent */
static inline unsigned int twi_get_stop(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val >>= 4;
	return reg_val & 1;
}

static inline void twi_disable_ack(void __iomem  *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val &= ~TWI_CTL_ACK;
	reg_val &= ~TWI_CTL_INTFLG;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

/* when sending ack or nack, it will send ack automatically */
static inline void twi_enable_ack(void __iomem  *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	reg_val |= TWI_CTL_ACK;
	reg_val &= ~TWI_CTL_INTFLG;
	writel(reg_val, base_addr + TWI_CTL_REG);
}

/* get the interrupt flag */
static inline unsigned int twi_query_irq_flag(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CTL_REG);
	return (reg_val & TWI_CTL_INTFLG);//0x 0000_1000
}

/* get interrupt status */
static inline unsigned int twi_query_irq_status(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_STAT_REG);
	return (reg_val & TWI_STAT_MASK);
}

/* set twi clock
 *
 * clk_n: clock divider factor n
 * clk_m: clock divider factor m
 */
static inline void twi_clk_write_reg(unsigned int clk_n, unsigned int clk_m, void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_CLK_REG);
	I2C_DBG("%s: clk_n = %d, clk_m = %d\n", __FUNCTION__, clk_n, clk_m);
	reg_val &= ~(TWI_CLK_DIV_M | TWI_CLK_DIV_N);
	reg_val |= ( clk_n |(clk_m << 3) );
	writel(reg_val, base_addr + TWI_CLK_REG);
}

/*
* Fin is APB CLOCK INPUT;
* Fsample = F0 = Fin/2^CLK_N;
* F1 = F0/(CLK_M+1);
* Foscl = F1/10 = Fin/(2^CLK_N * (CLK_M+1)*10);
* Foscl is clock SCL;100KHz or 400KHz
*
* clk_in: apb clk clock
* sclk_req: freqence to set in HZ
*/
static void twi_set_clock(unsigned int clk_in, unsigned int sclk_req, void __iomem *base_addr)
{
	unsigned int clk_m = 0;
	unsigned int clk_n = 0;
	unsigned int _2_pow_clk_n = 1;
	unsigned int src_clk      = clk_in/10;
	unsigned int divider      = src_clk/sclk_req;  // 400khz or 100khz
	unsigned int sclk_real    = 0;      // the real clock frequency

	if (divider == 0) {
		clk_m = 1;
		goto set_clk;
	}

	/* search clk_n and clk_m,from large to small value so that can quickly find suitable m & n. */
	while (clk_n < 8) { // 3bits max value is 8
		/* (m+1)*2^n = divider -->m = divider/2^n -1 */
		clk_m = (divider/_2_pow_clk_n) - 1;
		/* clk_m = (divider >> (_2_pow_clk_n>>1))-1 */
		while (clk_m < 16) { /* 4bits max value is 16 */
			sclk_real = src_clk/(clk_m + 1)/_2_pow_clk_n;  /* src_clk/((m+1)*2^n) */
			if (sclk_real <= sclk_req) {
				goto set_clk;
			} else {
				clk_m++;
			}
		}
		clk_n++;
		_2_pow_clk_n *= 2; /* mutilple by 2 */
	}

set_clk:
	twi_clk_write_reg(clk_n, clk_m, base_addr);

	return;
}

/* soft reset twi */
static inline void twi_soft_reset(void __iomem *base_addr)
{
	unsigned int reg_val = readl(base_addr + TWI_SRST_REG);
	reg_val |= TWI_SRST_SRST;
	writel(reg_val, base_addr + TWI_SRST_REG);
}

/* Enhanced Feature Register */
static inline void twi_set_efr(void __iomem *base_addr, unsigned int efr)
{
	unsigned int reg_val = readl(base_addr + TWI_EFR_REG);

	reg_val &= ~TWI_EFR_MASK;
	efr     &= TWI_EFR_MASK;
	reg_val |= efr;
	writel(reg_val, base_addr + TWI_EFR_REG);
}

static int sunxi_i2c_xfer_complete(struct sunxi_i2c *i2c, int code);
static int sunxi_i2c_do_xfer(struct sunxi_i2c *i2c, struct i2c_msg *msgs, int num);

static int twi_chan_is_enable(int _ch)
{
	return twi_used_mask & SUNXI_TWI_CHAN_MASK(_ch);
}

static int twi_select_gpio_state(struct pinctrl *pctrl, char *name, u32 no)
{
	int ret = 0;
	struct pinctrl_state *pctrl_state = NULL;

	pctrl_state = pinctrl_lookup_state(pctrl, name);
	if (IS_ERR(pctrl_state)) {
		I2C_ERR("TWI%d pinctrl_lookup_state(%s) failed! return %p \n", no, name, pctrl_state);
		return -1;
	}

	ret = pinctrl_select_state(pctrl, pctrl_state);
	if (ret < 0)
		I2C_ERR("TWI%d pinctrl_select_state(%s) failed! return %d \n", no, name, ret);

	return ret;
}

static int twi_request_gpio(struct sunxi_i2c *i2c)
{
	I2C_DBG("Pinctrl init %d ... [%s]\n", i2c->bus_num, i2c->adap.dev.parent->init_name);

	if (!twi_chan_is_enable(i2c->bus_num))
		return -1;
	
	i2c->pctrl = devm_pinctrl_get(i2c->adap.dev.parent);
	if (IS_ERR(i2c->pctrl)) {
		I2C_ERR("TWI%d devm_pinctrl_get() failed! return %ld\n", i2c->bus_num, PTR_ERR(i2c->pctrl));
		return -1;
	}

	return twi_select_gpio_state(i2c->pctrl, PINCTRL_STATE_DEFAULT, i2c->bus_num);
}

static void twi_release_gpio(struct sunxi_i2c *i2c)
{
	devm_pinctrl_put(i2c->pctrl);
}

/* function  */
static int twi_start(void __iomem *base_addr, int bus_num)
{
	unsigned int timeout = 0xff;

	twi_set_start(base_addr);
	while((1 == twi_get_start(base_addr))&&(--timeout));
	if (timeout == 0) {
		I2C_ERR("[i2c%d] START can't sendout!\n", bus_num);
		return SUNXI_I2C_FAIL;
	}

	return SUNXI_I2C_OK;
}

static int twi_restart(void __iomem *base_addr, int bus_num)
{
	unsigned int timeout = 0xff;
	twi_set_start(base_addr);
	twi_clear_irq_flag(base_addr);
	while((1 == twi_get_start(base_addr))&&(--timeout));
	if (timeout == 0) {
		I2C_ERR("[i2c%d] Restart can't sendout!\n", bus_num);
		return SUNXI_I2C_FAIL;
	}
	return SUNXI_I2C_OK;
}

static int twi_stop(void __iomem *base_addr, int bus_num)
{
	unsigned int timeout = 0xff;

	twi_set_stop(base_addr);
	twi_clear_irq_flag(base_addr);

	twi_get_stop(base_addr);/* it must delay 1 nop to check stop bit */
	while(( 1 == twi_get_stop(base_addr))&& (--timeout));
	if (timeout == 0) {
		I2C_ERR("[i2c%d] STOP can't sendout!\n", bus_num);
		return SUNXI_I2C_TFAIL;
	}

	timeout = 0xff;
	while((TWI_STAT_IDLE != readl(base_addr + TWI_STAT_REG))&&(--timeout));
	if (timeout == 0) {
		I2C_ERR("[i2c%d] i2c state(0x%08x) isn't idle(0xf8)\n", bus_num, readl(base_addr + TWI_STAT_REG));
		return SUNXI_I2C_TFAIL;
	}

	timeout = 0xff;
	while((TWI_LCR_IDLE_STATUS != readl(base_addr + TWI_LCR_REG))&&(--timeout));
	if (timeout == 0) {
		I2C_ERR("[i2c%d] i2c lcr(0x%08x) isn't idle(0x3a)\n", bus_num, readl(base_addr + TWI_LCR_REG));
		return SUNXI_I2C_TFAIL;
	}

	return SUNXI_I2C_OK;
}

/* get SDA state */
static unsigned int twi_get_sda(void __iomem *base_addr)
{
	unsigned int status = 0;
	status = TWI_LCR_SDA_STATE_MASK & readl(base_addr + TWI_LCR_REG);
	status >>= 4;
	return (status&0x1);
}

/* set SCL level(high/low), only when SCL enable */
static void twi_set_scl(void __iomem *base_addr, unsigned int hi_lo)
{
	unsigned int reg_val = readl(base_addr + TWI_LCR_REG);
	reg_val &= ~TWI_LCR_SCL_CTL;
	hi_lo   &= 0x01;
	reg_val |= (hi_lo<<3);
	writel(reg_val, base_addr + TWI_LCR_REG);
}

/* enable SDA or SCL */
static void twi_enable_lcr(void __iomem *base_addr, unsigned int sda_scl)
{
	unsigned int reg_val = readl(base_addr + TWI_LCR_REG);
	sda_scl &= 0x01;
	if (sda_scl)
		reg_val |= TWI_LCR_SCL_EN;/* enable scl line control */
	else
		reg_val |= TWI_LCR_SDA_EN;/* enable sda line control */

	writel(reg_val, base_addr + TWI_LCR_REG);
}

/* disable SDA or SCL */
static void twi_disable_lcr(void __iomem *base_addr, unsigned int sda_scl)
{
	unsigned int reg_val = readl(base_addr + TWI_LCR_REG);
	sda_scl &= 0x01;
	if (sda_scl)
		reg_val &= ~TWI_LCR_SCL_EN;/* disable scl line control */
	else
		reg_val &= ~TWI_LCR_SDA_EN;/* disable sda line control */

	writel(reg_val, base_addr + TWI_LCR_REG);
}

/* send 9 clock to release sda */
static int twi_send_clk_9pulse(void __iomem *base_addr, int bus_num)
{
	int twi_scl = 1;
	int low = 0;
	int high = 1;
	int cycle = 0;

	/* enable scl control */
	twi_enable_lcr(base_addr, twi_scl);

	while (cycle < 9){
		if (twi_get_sda(base_addr)
		    && twi_get_sda(base_addr)
		    && twi_get_sda(base_addr)) {
			break;
		}
		/* twi_scl -> low */
		twi_set_scl(base_addr, low);
		udelay(1000);

		/* twi_scl -> high */
		twi_set_scl(base_addr, high);
		udelay(1000);
		cycle++;
	}

	if (twi_get_sda(base_addr)) {
		twi_disable_lcr(base_addr, twi_scl);
		return SUNXI_I2C_OK;
	} else {
		I2C_ERR("[i2c%d] SDA is still Stuck Low, failed. \n", bus_num);
		twi_disable_lcr(base_addr, twi_scl);
		return SUNXI_I2C_FAIL;
	}
}

static int twi_regulator_request(struct sunxi_i2c_platform_data *pdata)
{
	struct regulator *regu = NULL;

	/* Consider "n***" as nocare. Support "none", "nocare", "null", "" etc. */
	if ((pdata->regulator_id[0] == 'n') || (pdata->regulator_id[0] == 0))
		return 0;

	regu = regulator_get(NULL, pdata->regulator_id);
	if (IS_ERR(regu)) {
		I2C_ERR("[i2c%d] get regulator %s failed!\n", pdata->bus_num, pdata->regulator_id);
		return -1;
	}
	pdata->regulator = regu;
	return 0;
}

static void twi_regulator_release(struct sunxi_i2c_platform_data *pdata)
{
	if (pdata->regulator == NULL)
		return;

	regulator_put(pdata->regulator);
	pdata->regulator = NULL;
}

static int twi_regulator_enable(struct sunxi_i2c_platform_data *pdata)
{
	if (pdata->regulator == NULL)
		return 0;

	if (regulator_enable(pdata->regulator) != 0) {
		I2C_ERR("[i2c%d] enable regulator %s failed!\n", pdata->bus_num, pdata->regulator_id);
		return -1;
	}
	return 0;
}

static int twi_regulator_disable(struct sunxi_i2c_platform_data *pdata)
{
	if (pdata->regulator == NULL)
		return 0;

	if (regulator_disable(pdata->regulator) != 0) {
		I2C_ERR("[i2c%d] enable regulator %s failed!\n", pdata->bus_num, pdata->regulator_id);
		return -1;
	}
	return 0;
}

/*
****************************************************************************************************
*
*  FunctionName:           sunxi_i2c_addr_byte
*
*  Description:
*            ����slave��ַ��7bit��ȫ����Ϣ����10bit�ĵ�һ���ֵ�ַ�����ⲿ�ӿڵ��ã��ڲ�ʵ�֡�
*         7bits addr: 7-1bits addr+0 bit r/w
*         10bits addr: 1111_11xx_xxxx_xxxx-->1111_0xx_rw,xxxx_xxxx
*         send the 7 bits addr,or the first part of 10 bits addr
*  Parameters:
*
*
*  Return value:
*           ��
*  Notes:
*
****************************************************************************************************
*/
static void sunxi_i2c_addr_byte(struct sunxi_i2c *i2c)
{
	unsigned char addr = 0;
	unsigned char tmp  = 0;

	if (i2c->msg[i2c->msg_idx].flags & I2C_M_TEN) {
		/* 0111_10xx,ten bits address--9:8bits */
		tmp = 0x78 | ( ( (i2c->msg[i2c->msg_idx].addr)>>8 ) & 0x03);
		addr = tmp << 1;//1111_0xx0
		/* how about the second part of ten bits addr? Answer: deal at twi_core_process() */
	} else {
		/* 7-1bits addr, xxxx_xxx0 */
		addr = (i2c->msg[i2c->msg_idx].addr & 0x7f) << 1;
	}

	/* read, default value is write */
	if (i2c->msg[i2c->msg_idx].flags & I2C_M_RD) {
		addr |= 1;
	}

#ifdef CONFIG_SUNXI_I2C_PRINT_TRANSFER_INFO
	if (i2c->bus_num == CONFIG_SUNXI_I2C_PRINT_TRANSFER_INFO_WITH_BUS_NUM) {
		if (i2c->msg[i2c->msg_idx].flags & I2C_M_TEN) {
			I2C_DBG("[i2c%d] first part of 10bits = 0x%x\n", i2c->bus_num, addr);
		}
		I2C_DBG("[i2c%d] 7bits+r/w = 0x%x\n", i2c->bus_num, addr);
	}
#else
	if (unlikely(bus_transfer_dbg != -1)) {
		if (i2c->bus_num == bus_transfer_dbg) {
			if (i2c->msg[i2c->msg_idx].flags & I2C_M_TEN) {
				I2C_DBG("[i2c%d] first part of 10bits = 0x%x\n", i2c->bus_num, addr);
			}
			I2C_DBG("[i2c%d] 7bits+r/w = 0x%x\n", i2c->bus_num, addr);
		}
	}
#endif
	/* send 7bits+r/w or the first part of 10bits */
	twi_put_byte(i2c->base_addr, &addr);
}


static int sunxi_i2c_core_process(struct sunxi_i2c *i2c)
{
	void __iomem *base_addr = i2c->base_addr;
	int  ret        = SUNXI_I2C_OK;
	int  err_code   = 0;
	unsigned char  state = 0;
	unsigned char  tmp   = 0;
	unsigned long flags = 0;

	state = twi_query_irq_status(base_addr);

	spin_lock_irqsave(&i2c->lock, flags);

#ifdef CONFIG_SUNXI_I2C_PRINT_TRANSFER_INFO
	if (i2c->bus_num == CONFIG_SUNXI_I2C_PRINT_TRANSFER_INFO_WITH_BUS_NUM) {
		I2C_DBG("[i2c%d][slave address = (0x%x), state = (0x%x)]\n", i2c->bus_num, i2c->msg->addr, state);
	}
#else
	if (unlikely(bus_transfer_dbg != -1)) {
		if (i2c->bus_num == bus_transfer_dbg) {
			I2C_DBG("[i2c%d][slave address = (0x%x), state = (0x%x)]\n", i2c->bus_num, i2c->msg->addr, state);
		}
	}
#endif

	if (i2c->msg == NULL) {
		I2C_ERR("[i2c%d] i2c message is NULL, err_code = 0xfe\n", i2c->bus_num);
		err_code = 0xfe;
		goto msg_null;
	}

	switch (state) {
	case 0xf8: /* On reset or stop the bus is idle, use only at poll method */
		err_code = 0xf8;
		goto err_out;
	case 0x08: /* A START condition has been transmitted */
	case 0x10: /* A repeated start condition has been transmitted */
		sunxi_i2c_addr_byte(i2c);/* send slave address */
		break;
	case 0xd8: /* second addr has transmitted, ACK not received!    */
	case 0x20: /* SLA+W has been transmitted; NOT ACK has been received */
		err_code = 0x20;
		goto err_out;
	case 0x18: /* SLA+W has been transmitted; ACK has been received */
		/* if any, send second part of 10 bits addr */
		if (i2c->msg[i2c->msg_idx].flags & I2C_M_TEN) {
			tmp = i2c->msg[i2c->msg_idx].addr & 0xff;  /* the remaining 8 bits of address */
			twi_put_byte(base_addr, &tmp); /* case 0xd0: */
			break;
		}
		/* for 7 bit addr, then directly send data byte--case 0xd0:  */
	case 0xd0: /* second addr has transmitted,ACK received!     */
	case 0x28: /* Data byte in DATA REG has been transmitted; ACK has been received */
		/* after send register address then START send write data  */
		if (i2c->msg_ptr < i2c->msg[i2c->msg_idx].len) {
			twi_put_byte(base_addr, &(i2c->msg[i2c->msg_idx].buf[i2c->msg_ptr]));
			i2c->msg_ptr++;
			break;
		}

		i2c->msg_idx++; /* the other msg */
		i2c->msg_ptr = 0;
		if (i2c->msg_idx == i2c->msg_num) {
			err_code = SUNXI_I2C_OK;/* Success,wakeup */
			goto ok_out;
		} else if (i2c->msg_idx < i2c->msg_num) {/* for restart pattern */
			ret = twi_restart(base_addr, i2c->bus_num);/* read spec, two msgs */
			if (ret == SUNXI_I2C_FAIL) {
				err_code = SUNXI_I2C_SFAIL;
				goto err_out;/* START can't sendout */
			}
		} else {
			err_code = SUNXI_I2C_FAIL;
			goto err_out;
		}
		break;
	case 0x30: /* Data byte in I2CDAT has been transmitted; NOT ACK has been received */
		err_code = 0x30;//err,wakeup the thread
		goto err_out;
	case 0x38: /* Arbitration lost during SLA+W, SLA+R or data bytes */
		err_code = 0x38;//err,wakeup the thread
		goto err_out;
	case 0x40: /* SLA+R has been transmitted; ACK has been received */
		/* with Restart,needn't to send second part of 10 bits addr,refer-"I2C-SPEC v2.1" */
		/* enable A_ACK need it(receive data len) more than 1. */
		if (i2c->msg[i2c->msg_idx].len > 1) {
			/* send register addr complete,then enable the A_ACK and get ready for receiving data */
			twi_enable_ack(base_addr);
			twi_clear_irq_flag(base_addr);/* jump to case 0x50 */
		} else if (i2c->msg[i2c->msg_idx].len == 1) {
			twi_clear_irq_flag(base_addr);/* jump to case 0x58 */
		}
		break;
	case 0x48: /* SLA+R has been transmitted; NOT ACK has been received */
		err_code = 0x48;//err,wakeup the thread
		goto err_out;
	case 0x50: /* Data bytes has been received; ACK has been transmitted */
		/* receive first data byte */
		if (i2c->msg_ptr < i2c->msg[i2c->msg_idx].len) {
			/* more than 2 bytes, the last byte need not to send ACK */
			if ((i2c->msg_ptr + 2) == i2c->msg[i2c->msg_idx].len ) {
				twi_disable_ack(base_addr);/* last byte no ACK */
			}
			/* get data then clear flag,then next data comming */
			twi_get_byte(base_addr, &i2c->msg[i2c->msg_idx].buf[i2c->msg_ptr]);
			i2c->msg_ptr++;
			break;
		}
		/* err process, the last byte should be @case 0x58 */
		err_code = SUNXI_I2C_FAIL;/* err, wakeup */
		goto err_out;
	case 0x58: /* Data byte has been received; NOT ACK has been transmitted */
		/* received the last byte  */
		if ( i2c->msg_ptr == i2c->msg[i2c->msg_idx].len - 1 ) {
			twi_get_last_byte(base_addr, &i2c->msg[i2c->msg_idx].buf[i2c->msg_ptr]);
			i2c->msg_idx++;
			i2c->msg_ptr = 0;
			if (i2c->msg_idx == i2c->msg_num) {
				err_code = SUNXI_I2C_OK; // succeed,wakeup the thread
				goto ok_out;
			} else if (i2c->msg_idx < i2c->msg_num) {
				/* repeat start */
				ret = twi_restart(base_addr, i2c->bus_num);
				if(ret == SUNXI_I2C_FAIL) {/* START fail */
					err_code = SUNXI_I2C_SFAIL;
					goto err_out;
				}
				break;
			}
		} else {
			err_code = 0x58;
			goto err_out;
		}
	case 0x00: /* Bus error during master or slave mode due to illegal level condition */
		err_code = 0xff;
		goto err_out;
	default:
		err_code = state;
		goto err_out;
	}
	i2c->debug_state = state;/* just for debug */
	spin_unlock_irqrestore(&i2c->lock, flags);
	return ret;

ok_out:
err_out:
	if (SUNXI_I2C_TFAIL == twi_stop(base_addr, i2c->bus_num)) {
		I2C_ERR("[i2c%d] STOP failed!\n", i2c->bus_num);
	}

msg_null:
	ret = sunxi_i2c_xfer_complete(i2c, err_code);/* wake up */
	i2c->debug_state = state;/* just for debug */
	spin_unlock_irqrestore(&i2c->lock, flags);
	return ret;
}

static irqreturn_t sunxi_i2c_handler(int this_irq, void * dev_id)
{
	struct sunxi_i2c *i2c = (struct sunxi_i2c *)dev_id;

	if (!twi_query_irq_flag(i2c->base_addr)) {
		I2C_ERR("unknown interrupt!\n");
		return IRQ_NONE;
	}

	/* disable irq */
	twi_disable_irq(i2c->base_addr);

	/* twi core process */
	sunxi_i2c_core_process(i2c);

	/* enable irq only when twi is transfering, otherwise disable irq */
	if (i2c->status != I2C_XFER_IDLE) {
		twi_enable_irq(i2c->base_addr);
	}

	return IRQ_HANDLED;
}

static int sunxi_i2c_xfer_complete(struct sunxi_i2c *i2c, int code)
{
	int ret = SUNXI_I2C_OK;

	i2c->msg     = NULL;
	i2c->msg_num = 0;
	i2c->msg_ptr = 0;
	i2c->status  = I2C_XFER_IDLE;

	/* i2c->msg_idx  store the information */
	if (code == SUNXI_I2C_FAIL) {
		I2C_ERR("[i2c%d] Maybe Logic Error, debug it!\n", i2c->bus_num);
		i2c->msg_idx = code;
		ret = SUNXI_I2C_FAIL;
	} else if (code != SUNXI_I2C_OK) {
		i2c->msg_idx = code;
		ret = SUNXI_I2C_FAIL;
	}

	wake_up(&i2c->wait);

	return ret;
}

static int sunxi_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct sunxi_i2c *i2c = (struct sunxi_i2c *)adap->algo_data;
	int ret = SUNXI_I2C_FAIL;
	int i   = 0;

	if (i2c->suspended) {
		I2C_ERR("[i2c%d] has already suspend, dev addr:0x%x!\n", i2c->adap.nr, msgs->addr);
		return -ENODEV;
	}

	for (i = 1; i <= adap->retries; i++) {
		ret = sunxi_i2c_do_xfer(i2c, msgs, num);

		if (ret != SUNXI_I2C_RETRY) {
			goto out;
		}

		I2C_DBG("[i2c%d] Retrying transmission %d\n", i2c->adap.nr, i);
		udelay(100);
	}

	ret = -EREMOTEIO;
out:
	return ret;
}

static int sunxi_i2c_do_xfer(struct sunxi_i2c *i2c, struct i2c_msg *msgs, int num)
{
	unsigned long timeout = 0;
	int ret = SUNXI_I2C_FAIL;
	unsigned long flags = 0;
	//int i = 0, j =0;

	twi_soft_reset(i2c->base_addr);
	udelay(100);

	/* test the bus is free,already protect by the semaphore at DEV layer */
	while (TWI_STAT_IDLE != twi_query_irq_status(i2c->base_addr)&&
	       TWI_STAT_BUS_ERR != twi_query_irq_status(i2c->base_addr) &&
	       TWI_STAT_ARBLOST_SLAR_ACK != twi_query_irq_status(i2c->base_addr)) {
		I2C_DBG("[i2c%d] bus is busy, status = %x\n", i2c->bus_num, twi_query_irq_status(i2c->base_addr));
		if (SUNXI_I2C_OK == twi_send_clk_9pulse(i2c->base_addr, i2c->bus_num)) {
			break;
		} else {
			ret = SUNXI_I2C_RETRY;
			goto out;
		}
	}

	/* may conflict with xfer_complete */
	spin_lock_irqsave(&i2c->lock, flags);
	i2c->msg     = msgs;
	i2c->msg_num = num;
	i2c->msg_ptr = 0;
	i2c->msg_idx = 0;
	i2c->status  = I2C_XFER_START;
	twi_enable_irq(i2c->base_addr);  /* enable irq */
	twi_disable_ack(i2c->base_addr); /* disabe ACK */
	twi_set_efr(i2c->base_addr, 0);  /* set the special function register,default:0. */
	spin_unlock_irqrestore(&i2c->lock, flags);

	/* START signal, needn't clear int flag */
	ret = twi_start(i2c->base_addr, i2c->bus_num);
	if (ret == SUNXI_I2C_FAIL) {
		twi_soft_reset(i2c->base_addr);
		twi_disable_irq(i2c->base_addr);  /* disable irq */
		i2c->status  = I2C_XFER_IDLE;
		ret = SUNXI_I2C_RETRY;
		goto out;
	}

	i2c->status  = I2C_XFER_RUNNING;
	/* sleep and wait, do the transfer at interrupt handler ,timeout = 5*HZ */
	timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, i2c->adap.timeout);
	/* return code,if(msg_idx == num) succeed */
	ret = i2c->msg_idx;
	if (timeout == 0) {
		I2C_ERR("[i2c%d] xfer timeout (dev addr:0x%x)\n", i2c->bus_num, msgs->addr);
		spin_lock_irqsave(&i2c->lock, flags);
		i2c->msg = NULL;
		spin_unlock_irqrestore(&i2c->lock, flags);
		ret = -ETIME;
	} else if (ret != num) {
		I2C_ERR("[i2c%d] incomplete xfer (status: 0x%x, dev addr: 0x%x)\n", i2c->bus_num, ret, msgs->addr);
		ret = -ECOMM;
	}
out:
	return ret;
}

static unsigned int sunxi_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C|I2C_FUNC_10BIT_ADDR|I2C_FUNC_SMBUS_EMUL;
}


static const struct i2c_algorithm sunxi_i2c_algorithm = {
	.master_xfer	  = sunxi_i2c_xfer,
	.functionality	  = sunxi_i2c_functionality,
};

#ifdef CONFIG_EVB_PLATFORM

static int sunxi_i2c_clk_init(struct sunxi_i2c *i2c)
{
	unsigned int apb_clk = 0;

	if (clk_prepare_enable(i2c->mclk)) {
		I2C_ERR("[i2c%d] enable apb_twi clock failed!\n", i2c->bus_num);
		return -1;
	}

	//sunxi_periph_reset_deassert(i2c->mclk);

	/* enable twi bus */
	twi_enable_bus(i2c->base_addr);

	/* set twi module clock */
	apb_clk  =  clk_get_rate(i2c->mclk);
	if (apb_clk == 0) {
		I2C_ERR("[i2c%d] get i2c source clock frequency failed!\n", i2c->bus_num);
		return -1;
	}

	twi_set_clock(apb_clk, i2c->bus_freq, i2c->base_addr);

	return 0;
}

static int sunxi_i2c_clk_exit(struct sunxi_i2c *i2c)
{
	/* disable twi bus */
	twi_disable_bus(i2c->base_addr);

	/* disable clk */
	if (IS_ERR_OR_NULL(i2c->mclk)) {
		I2C_ERR("[i2c%d] i2c mclk handle is invalid, just return!\n", i2c->bus_num);
		return -1;
	} else {
//		sunxi_periph_reset_assert(i2c->mclk);
	
		clk_disable_unprepare(i2c->mclk);
	}

	return 0;
}

#else

static int sunxi_i2c_clk_init(struct sunxi_i2c *i2c)
{
	twi_enable_bus(i2c->base_addr);
	
	twi_set_clock(24000000, i2c->bus_freq, i2c->base_addr);
	return 0;
}

static int sunxi_i2c_clk_exit(struct sunxi_i2c *i2c)
{
	twi_disable_bus(i2c->base_addr);

	return 0;
}
#endif

static int sunxi_i2c_hw_init(struct sunxi_i2c *i2c, struct sunxi_i2c_platform_data *pdata)
{
	int ret = 0;

	ret = twi_regulator_request(pdata);
	if (ret < 0) {
		I2C_ERR("[i2c%d] request regulator failed!\n", i2c->bus_num);
		return -1;
	}
	twi_regulator_enable(pdata);

	ret = twi_request_gpio(i2c);
	if (ret < 0) {
		I2C_ERR("[i2c%d] request i2c gpio failed!\n", i2c->bus_num);
		return -1;
	}

	if (sunxi_i2c_clk_init(i2c)) {
		I2C_ERR("[i2c%d] init i2c clock failed!\n", i2c->bus_num);
		return -1;
	}

	twi_soft_reset(i2c->base_addr);

	return ret;
}

static void sunxi_i2c_hw_exit(struct sunxi_i2c *i2c, struct sunxi_i2c_platform_data *pdata)
{
	if (sunxi_i2c_clk_exit(i2c)) {
		I2C_ERR("[i2c%d] exit i2c clock failed!\n", i2c->bus_num);
		return;
	}
	twi_release_gpio(i2c);

	twi_regulator_disable(pdata);
	twi_regulator_release(pdata);
}

static void sunxi_i2c_release(struct device *dev)
{
	I2C_ENTER();
}

static ssize_t sunxi_i2c_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = container_of(dev, struct platform_device, dev);
	struct sunxi_i2c_platform_data *pdata = dev->platform_data;

	return snprintf(buf, PAGE_SIZE,
	   	"pdev->id   = %d \n"
	   	"pdev->name = %s \n"
	   	"pdev->num_resources = %u \n"
		"pdev->resource.mem = [%pa, %pa] \n"
		"pdev->resource.irq = %pa \n"
	   	"pdev->dev.platform_data.bus_num  = %d \n"
		"pdev->dev.platform_data.freqency = %d \n"
		"pdev->dev.platform_data.regulator= 0x%p \n"
		"pdev->dev.platform_data.regulator_id = %s \n",
		pdev->id, pdev->name, pdev->num_resources,
		&pdev->resource[0].start, &pdev->resource[0].end, &pdev->resource[1].start,
		pdata->bus_num, pdata->frequency, pdata->regulator, pdata->regulator_id);
}
static struct device_attribute sunxi_i2c_info_attr =
	__ATTR(info, S_IRUGO, sunxi_i2c_info_show, NULL);

static ssize_t sunxi_i2c_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sunxi_i2c *i2c = dev_get_drvdata(dev);
	char *i2c_status[] = {"Unknown", "Idle", "Start", "Unknown", "Running"};

	if (i2c == NULL)
		return snprintf(buf, PAGE_SIZE, "%s\n", "sunxi_i2c is NULL!");

	return snprintf(buf, PAGE_SIZE, 
	   	"i2c->bus_num = %d \n"
	   	"i2c->status  = [%d] %s \n"
	   	"i2c->suspended = %d \n"
	   	"i2c->msg_num   = %d, ->msg_idx = %d, ->msg_ptr = %d \n"
	   	"i2c->bus_freq  = %d \n"
	   	"i2c->irq       = %d \n"
	   	"i2c->debug_state = %d \n"
	   	"i2c->base_addr = 0x%p, the TWI control register: \n"
	   	"[ADDR] 0x%02x = 0x%08x, [XADDR] 0x%02x = 0x%08x, [DATA] 0x%02x = 0x%08x \n"
	   	"[CNTR] 0x%02x = 0x%08x, [STAT]  0x%02x = 0x%08x, [CCR]  0x%02x = 0x%08x \n"
	   	"[SRST] 0x%02x = 0x%08x, [EFR]   0x%02x = 0x%08x, [LCR]  0x%02x = 0x%08x \n",
	   	i2c->bus_num, i2c->status, i2c_status[i2c->status],
	   	i2c->suspended,
	   	i2c->msg_num, i2c->msg_idx, i2c->msg_ptr,
	   	i2c->bus_freq, i2c->irq, i2c->debug_state,
	   	i2c->base_addr,
		TWI_ADDR_REG,  readl(i2c->base_addr + TWI_ADDR_REG),
		TWI_XADDR_REG, readl(i2c->base_addr + TWI_XADDR_REG),
		TWI_DATA_REG,  readl(i2c->base_addr + TWI_DATA_REG),
		TWI_CTL_REG,   readl(i2c->base_addr + TWI_CTL_REG),
		TWI_STAT_REG,  readl(i2c->base_addr + TWI_STAT_REG),
		TWI_CLK_REG,   readl(i2c->base_addr + TWI_CLK_REG),
		TWI_SRST_REG,  readl(i2c->base_addr + TWI_SRST_REG),
		TWI_EFR_REG,   readl(i2c->base_addr + TWI_EFR_REG),
		TWI_LCR_REG,   readl(i2c->base_addr + TWI_LCR_REG));
}
static struct device_attribute sunxi_i2c_status_attr =
	__ATTR(status, S_IRUGO, sunxi_i2c_status_show, NULL);

static void ut_twi_dev_scan(void)
{
}

static ut_case_t gs_twi_ut_case[] = {
	{"twi_dev_scan", NULL, ut_twi_dev_scan, NULL},
};

static ssize_t sunxi_i2c_ut_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	UT_RUN(gs_twi_ut_case);
	UT_END();

	return UT_RESULT(buf, PAGE_SIZE);
}
static struct device_attribute sunxi_i2c_unittest_attr =
	__ATTR(unittest, S_IRUGO, sunxi_i2c_ut_show, NULL);

static void sunxi_i2c_sysfs(struct platform_device *_pdev)
{
	device_create_file(&_pdev->dev, &sunxi_i2c_info_attr);
	device_create_file(&_pdev->dev, &sunxi_i2c_status_attr);
	device_create_file(&_pdev->dev, &sunxi_i2c_unittest_attr);
}

static int sunxi_i2c_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node; 
	struct sunxi_i2c *i2c = NULL;
	struct resource *mem_res = NULL;
	struct sunxi_i2c_platform_data *pdata = NULL;
	int ret, irq;

	if (np == NULL) {
		I2C_ERR("I2C failed to get of node\n");
		return -ENODEV;
	}

	i2c = kzalloc(sizeof(struct sunxi_i2c), GFP_KERNEL);
	if (!i2c) {
		return -ENOMEM;
	}

	pdata = kzalloc(sizeof(struct sunxi_i2c_platform_data), GFP_KERNEL);
	if (pdata == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}
	pdev->dev.platform_data = pdata;

	pdev->id = of_alias_get_id(np, "twi");
	if (pdev->id < 0) {
		I2C_ERR("I2C failed to get alias id\n");
		ret = -EINVAL;
		goto emem;
	}
	pdata->bus_num  = pdev->id;

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		I2C_ERR("[I2C%d] failed to get MEM res\n", pdev->id);
		ret = -ENXIO;
		goto emem;
	}
	
	if (!request_mem_region(mem_res->start, resource_size(mem_res), mem_res->name)) {
		I2C_ERR("[I2C%d] failed to request mem region\n", pdev->id);
		ret = -EINVAL;
		goto emem;
	}

	i2c->base_addr = ioremap(mem_res->start, resource_size(mem_res));
	if (!i2c->base_addr) {
		ret = -EIO;
		goto eiomap;
	}
	
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		I2C_ERR("[I2C%d] failed to get irq\n", pdev->id);
		ret = -EINVAL;
		goto eiomap;
	}

	ret = of_property_read_u32(np, "clock-frequency", &pdata->frequency);
	if (ret) {
		I2C_ERR("[I2C%d] failed to get clock frequency\n", pdev->id);
		ret = -EINVAL;
		goto eiomap;
	}

	pdev->dev.release = sunxi_i2c_release;
	i2c->adap.owner   = THIS_MODULE;
	i2c->adap.nr      = pdata->bus_num;
	i2c->adap.retries = 3;
	i2c->adap.timeout = 5*HZ;
	i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	i2c->bus_freq     = pdata->frequency;
	i2c->irq          = irq;
	i2c->bus_num      = pdata->bus_num;
	i2c->status       = I2C_XFER_IDLE;
	i2c->suspended = 0;	
	snprintf(i2c->adap.name, sizeof(i2c->adap.name), SUNXI_TWI_DEV_NAME"%u", i2c->adap.nr);
	pdev->dev.init_name = i2c->adap.name;
	
	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

#ifdef CONFIG_EVB_PLATFORM
	i2c->mclk = of_clk_get(np, 0);
	if (IS_ERR_OR_NULL(i2c->mclk)) {
		I2C_ERR("[i2c%d] request TWI clock failed\n", i2c->bus_num);
		ret = -EIO;
		goto eremap;
	}
#endif

	i2c->adap.algo = &sunxi_i2c_algorithm;
	ret = request_irq(irq, sunxi_i2c_handler, IRQF_DISABLED, i2c->adap.name, i2c);
	if (ret) {
		I2C_ERR("[i2c%d] requeset irq failed!\n", i2c->bus_num);
		goto ereqirq;
	}

	i2c->adap.algo_data  = i2c;
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.dev.of_node = pdev->dev.of_node;
	twi_used_mask |= SUNXI_TWI_CHAN_MASK(pdev->id);

	if (sunxi_i2c_hw_init(i2c, pdata)) {
		ret = -EIO;
		goto ehwinit;
	}

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		I2C_ERR( "[i2c%d] failed to add adapter\n", i2c->bus_num);
		goto eadapt;
	}

	of_i2c_register_devices(&i2c->adap);
	platform_set_drvdata(pdev, i2c);

	sunxi_i2c_sysfs(pdev);

	I2C_DBG("I2C: %s: sunxi I2C adapter\n", dev_name(&i2c->adap.dev));
	I2C_DBG("TWI_CTL  0x%p: 0x%08x \n", i2c->base_addr + 0x0c, readl(i2c->base_addr + 0x0c));
	I2C_DBG("TWI_STAT 0x%p: 0x%08x \n", i2c->base_addr + 0x10, readl(i2c->base_addr + 0x10));
	I2C_DBG("TWI_CLK  0x%p: 0x%08x \n", i2c->base_addr + 0x14, readl(i2c->base_addr + 0x14));
	I2C_DBG("TWI_SRST 0x%p: 0x%08x \n", i2c->base_addr + 0x18, readl(i2c->base_addr + 0x18));
	I2C_DBG("TWI_EFR  0x%p: 0x%08x \n", i2c->base_addr + 0x1c, readl(i2c->base_addr + 0x1c));

	return 0;

eadapt:
#ifdef CONFIG_EVB_PLATFORM
	clk_disable_unprepare(i2c->mclk);
#endif

ehwinit:
	free_irq(irq, i2c);

ereqirq:
	iounmap(i2c->base_addr);

#ifdef CONFIG_EVB_PLATFORM
eremap:
	if (!IS_ERR_OR_NULL(i2c->mclk)) {
		clk_put(i2c->mclk);
		i2c->mclk = NULL;
	}
#endif

eiomap:
	release_mem_region(mem_res->start, resource_size(mem_res));

emem:
	kfree(pdata);
	kfree(i2c);

	return ret;
}

static int sunxi_i2c_remove(struct platform_device *pdev)
{
	struct sunxi_i2c *i2c = platform_get_drvdata(pdev);

	I2C_DBG("[i2c.%d] remove ... \n", i2c->bus_num);

	platform_set_drvdata(pdev, NULL);
	i2c_del_adapter(&i2c->adap);
	free_irq(i2c->irq, i2c);

	/* disable clock and release gpio */
	sunxi_i2c_hw_exit(i2c, pdev->dev.platform_data);
#ifdef CONFIG_EVB_PLATFORM
	if (IS_ERR_OR_NULL(i2c->mclk)) {
		I2C_ERR("i2c mclk handle is invalid, just return!\n");
		return -1;
	} else {
		clk_put(i2c->mclk);
		i2c->mclk = NULL;
	}
#endif

	iounmap(i2c->base_addr);
	kfree(i2c);
	kfree(pdev->dev.platform_data);

	I2C_EXIT();
	return 0;
}

#ifdef CONFIG_PM
static int sunxi_i2c_suspend(struct device *dev)
{
#ifdef CONFIG_EVB_PLATFORM
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_i2c *i2c = platform_get_drvdata(pdev);
	int count = 10;

	while ((i2c->status != I2C_XFER_IDLE) && (count-- > 0)) {
		I2C_ERR("[i2c%d] suspend while xfer,dev addr = 0x%x\n",
			i2c->adap.nr, i2c->msg? i2c->msg->addr : 0xff);
		msleep(100);
	}

	i2c->suspended = 1;

	if (sunxi_i2c_clk_exit(i2c)) {
		I2C_ERR("[i2c%d] suspend failed.. \n", i2c->bus_num);
		i2c->suspended = 0;
		return -1;
	}

	twi_select_gpio_state(i2c->pctrl, PINCTRL_STATE_SLEEP, i2c->bus_num);
	twi_regulator_disable(dev->platform_data);

	I2C_DBG("[i2c%d] suspend okay.. \n", i2c->bus_num);
#endif
	return 0;
}

static int sunxi_i2c_resume(struct device *dev)
{
#ifdef CONFIG_EVB_PLATFORM
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_i2c *i2c = platform_get_drvdata(pdev);

	i2c->suspended = 0;

	twi_regulator_enable(dev->platform_data);
	twi_select_gpio_state(i2c->pctrl, PINCTRL_STATE_DEFAULT, i2c->bus_num);

	if (sunxi_i2c_clk_init(i2c)) {
		I2C_ERR("[i2c%d] resume failed.. \n", i2c->bus_num);
		return -1;
	}

	twi_soft_reset(i2c->base_addr);

	I2C_DBG("[i2c%d] resume okay.. \n", i2c->bus_num);
#endif
	return 0;
}

static const struct dev_pm_ops sunxi_i2c_dev_pm_ops = {
	.suspend = sunxi_i2c_suspend,
	.resume = sunxi_i2c_resume,
};

#define SUNXI_I2C_DEV_PM_OPS (&sunxi_i2c_dev_pm_ops)
#else
#define SUNXI_I2C_DEV_PM_OPS NULL
#endif

static const struct of_device_id sunxi_i2c_match[] = {
	{ .compatible = "allwinner,sun8i-twi", },
	{ .compatible = "allwinner,sun50i-twi", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_i2c_match);

static struct platform_driver sunxi_i2c_driver = {
	.probe		= sunxi_i2c_probe,
	.remove		= sunxi_i2c_remove,
	.driver		= {
		.name	= SUNXI_TWI_DEV_NAME,
		.owner	= THIS_MODULE,
		.pm		= SUNXI_I2C_DEV_PM_OPS,
		.of_match_table = sunxi_i2c_match,
	},
};

static int __init sunxi_i2c_adap_init(void)
{
	I2C_DBG("Sunxi I2C adapt init\n");

	return platform_driver_register(&sunxi_i2c_driver);
}

static void __exit sunxi_i2c_adap_exit(void)
{
	
	I2C_ENTER();

	platform_driver_unregister(&sunxi_i2c_driver);
}

subsys_initcall(sunxi_i2c_adap_init);
module_exit(sunxi_i2c_adap_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:i2c-sunxi");
MODULE_DESCRIPTION("SUNXI I2C Bus Driver");
MODULE_AUTHOR("pannan");
