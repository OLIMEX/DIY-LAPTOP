/*
 * sunxi-scr.c smartcard driver
 *
 * Copyright (C) 2013-2014 allwinner.
 *	Ming Li<liming@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/major.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/sys_config.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <asm/irq.h>
#include "sunxi-scr.h"
#include "smartcard.h"
#include "sunxi-scr-common.h"

static scr_struct scr;
static scatr_struct scatr;
static upps_struct pps;
static atr_buffer atr_data;
static int sunxi_scr_major = -1;
static struct class *scr_dev_class;
static struct workqueue_struct *scr_wq = NULL;
static struct work_struct  scr_work;
static struct device *scr_device = NULL;
static char scr_dev_name[] = "sim0";
static struct pinctrl *scr_pinctrl = NULL;
static cardPara card_para = {0};
static DEFINE_SPINLOCK(rx_lock);
u32 scr_debug_mask = 0x0;

__inline uint32_t scr_buffer_is_empty(pscr_buffer pbuf)
{
	return (pbuf->wptr==pbuf->rptr);
}

__inline uint32_t scr_buffer_is_full(pscr_buffer pbuf)
{
	return ((pbuf->wptr^pbuf->rptr)==(SCR_BUFFER_SIZE_MASK+1));
}

__inline void scr_buffer_flush(pscr_buffer pbuf)
{
	pbuf->wptr = pbuf->rptr = 0;
}

static void scr_fill_buffer(pscr_buffer pbuf, uint8_t data)
{
	pbuf->buffer[pbuf->wptr&SCR_BUFFER_SIZE_MASK] = data;
	pbuf->wptr ++;
	pbuf->wptr &= (SCR_BUFFER_SIZE_MASK<<1)|0x1;    /* bit0:bit7 */
}

uint8_t scr_dump_buffer(pscr_buffer pbuf)
{
	uint8_t data;

	data = pbuf->buffer[pbuf->rptr&SCR_BUFFER_SIZE_MASK];
	pbuf->rptr ++;
	pbuf->rptr &= (SCR_BUFFER_SIZE_MASK<<1)|0x1;	/* bit0:bit7 */

	return data;
}

void scr_display_buffer_data(pscr_buffer pbuf)
{
	uint32_t i=0;

	if(!scr_buffer_is_empty(pbuf))
	{
		/* pbuf->rptr < pbuf->wptr */
		for(i=pbuf->rptr; (i&((SCR_BUFFER_SIZE_MASK<<1)|0x1)) != pbuf->wptr; i++)
		{
			dprintk(DEBUG_DATA_INFO, "0x%x ", pbuf->buffer[i&SCR_BUFFER_SIZE_MASK]);
		}
		dprintk(DEBUG_DATA_INFO, "\n");
	}
	else
	{
		dprintk(DEBUG_DATA_INFO, "Buffer is Empty when Display!!\n");
	}
}

void scr_copy_atr_data(pscr_buffer pbuf, atr_buffer *atr_data)
{
	uint32_t i=0;
	atr_data->atr_len = 0;

	if(!scr_buffer_is_empty(pbuf))
	{
		/* pbuf->rptr < pbuf->wptr */
		for(i=pbuf->rptr; (i&((SCR_BUFFER_SIZE_MASK<<1)|0x1)) != pbuf->wptr; i++)
		{
			atr_data->buffer[atr_data->atr_len] = pbuf->buffer[i&SCR_BUFFER_SIZE_MASK];
			dprintk(DEBUG_DATA_INFO, "0x%x ", atr_data->buffer[atr_data->atr_len]);
			atr_data->atr_len++;
		}
		dprintk(DEBUG_DATA_INFO, "\n");
	}
	else
	{
		printk(KERN_ERR "%s: Buffer is Empty when Display!!\n", __func__);
	}
}

static void scr_work_func(struct work_struct *work)
{
	if(scr.irq_accsta & SCR_INTSTA_INS)
	{
		scr.irq_accsta &= ~SCR_INTSTA_INS;
		scr.detected = 1;
		scr.atr_resp = SCR_ATR_RESP_INVALID;
		dprintk(DEBUG_INT, "\n\nSmartCard Inserted!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_REM)
	{
		scr.irq_accsta &= ~SCR_INTSTA_REM;
		scr.detected = 0;
		scr.atr_resp = SCR_ATR_RESP_INVALID;
		dprintk(DEBUG_INT, "SmartCard Removed!!\n\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_ACT)
	{
		scr.irq_accsta &= ~SCR_INTSTA_ACT;
		scr.activated = 1;
		scr.atr_resp = SCR_ATR_RESP_INVALID;
		dprintk(DEBUG_INT, "SmartCard Activated!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_DEACT)
	{
		scr.irq_accsta &= ~SCR_INTSTA_DEACT;
		scr.activated = 0;
		scr.atr_resp = SCR_ATR_RESP_INVALID;
		dprintk(DEBUG_INT, "SmartCard Deactivated!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_TXFDONE)
	{
		scr.irq_accsta &= ~SCR_INTSTA_TXFDONE;
		dprintk(DEBUG_INT, "SmartCard TxFIFO Done!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_TXFEMPTY)
	{
		scr.irq_accsta &= ~SCR_INTSTA_TXFEMPTY;
		dprintk(DEBUG_INT, "SmartCard TxFIFO Empty!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_TXFTH)
	{
		scr.irq_accsta &= ~SCR_INTSTA_TXFTH;
		dprintk(DEBUG_INT, "SmartCard TxFIFO Threshold!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_TXDONE)
	{
		scr.irq_accsta &= ~SCR_INTSTA_TXDONE;
		dprintk(DEBUG_INT, "SmartCard Tx Done!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_TXPERR)
	{
		scr.irq_accsta &= ~SCR_INTSTA_TXPERR;
		dprintk(DEBUG_INT, "SmartCard Tx Parity Error!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_RXFFULL)
	{
		scr.irq_accsta &= ~SCR_INTSTA_RXFFULL;
		dprintk(DEBUG_INT, "SmartCard Rx FIFO Full!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_RXPERR)
	{
		scr.irq_accsta &= ~SCR_INTSTA_RXPERR;
		dprintk(DEBUG_INT, "SmartCard Rx Parity Error!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_CLOCK)
	{
		scr.irq_accsta &= ~SCR_INTSTA_CLOCK;
		dprintk(DEBUG_INT, "SmartCard Stop/Start Clock!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_CHTO)
	{
		scr.irq_accsta &= ~SCR_INTSTA_CHTO;
		scr.chto_flag ++;
		dprintk(DEBUG_INT, "SmartCard Character Timeout!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_ATRFAIL)
	{
		scr.irq_accsta &= ~SCR_INTSTA_ATRFAIL;
		scr.atr_resp = SCR_ATR_RESP_FAIL;
		dprintk(DEBUG_INT, "SmartCard ATR Fail!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_ATRDONE)
	{
		unsigned long irqflags;

		scr.irq_accsta &= ~SCR_INTSTA_ATRDONE;
		spin_lock_irqsave(&rx_lock, irqflags);
		scr_copy_atr_data(&(scr.rxbuf), &atr_data);
		scr_buffer_flush(&scr.rxbuf);
		spin_unlock_irqrestore(&rx_lock, irqflags);

		smartcard_atr_decode(&scr, &scatr, (uint8_t*)atr_data.buffer, &pps, 1);
		scr.atr_resp = SCR_ATR_RESP_OK;
		dprintk(DEBUG_INT, "SmartCard ATR Done!!\n");
	}
	return;
}

/* data irq must immediate processing. */
static void irq_immediate(void)
{
	unsigned long irqflags;

	if(scr.irq_accsta & SCR_INTSTA_RXFTH)
	{
		scr.irq_accsta &= ~SCR_INTSTA_RXFTH;

		spin_lock_irqsave(&rx_lock, irqflags);
		if(!scr_buffer_is_full(&(scr.rxbuf)))
		{
			while(!scr_rxfifo_is_empty(&scr))
			{
				scr_fill_buffer(&(scr.rxbuf), scr_read_fifo(&scr));
			}
			spin_unlock_irqrestore(&rx_lock, irqflags);
		}
		else
		{
			spin_unlock_irqrestore(&rx_lock, irqflags);
			dprintk(DEBUG_INT, "SmartCard RxBuffer Overflow 1!!\n");
		}

		if (0 != scr_debug_mask)
			scr_display_buffer_data(&(scr.rxbuf));

		dprintk(DEBUG_INT, "SmartCard Rx FIFO Threshold First!!\n");
	}

	if(scr.irq_accsta & SCR_INTSTA_RXDONE)
	{
		scr.irq_accsta &= ~SCR_INTSTA_RXDONE;

		spin_lock_irqsave(&rx_lock, irqflags);
		if(!scr_buffer_is_full(&(scr.rxbuf)))
		{
			while(!scr_rxfifo_is_empty(&scr))
			{
				scr_fill_buffer(&(scr.rxbuf), scr_read_fifo(&scr));
			}
			spin_unlock_irqrestore(&rx_lock, irqflags);
		}
		else
		{
			spin_unlock_irqrestore(&rx_lock, irqflags);
			dprintk(DEBUG_INT, "SmartCard RxBuffer Overflow 2!!\n");
		}
		if (0 != scr_debug_mask)
			scr_display_buffer_data(&(scr.rxbuf));
		dprintk(DEBUG_INT, "SmartCard Rx Done!!\n");
	}

}

static irqreturn_t scr_irq_service(int irqno, void *dev_id)
{
	uint32_t temp = scr_get_interrupt_status(&scr);
	scr_clear_interrupt_status(&scr, temp);


	scr.irq_accsta |= temp;
	scr.irq_cursta = temp;

	dprintk(DEBUG_INT, "%s: enter!!scr.irq_accsta:%x  \n", __func__, scr.irq_accsta);
	if ((scr.irq_accsta & SCR_INTSTA_RXFTH) || (scr.irq_accsta & SCR_INTSTA_RXDONE))
		irq_immediate();
	else
	queue_work(scr_wq, &scr_work);
	return IRQ_HANDLED;
}

static int scr_request_gpio(void)
{
	int ret = 0;
	struct pinctrl_state *pctrl_state = NULL;

	if (NULL != scr.scr_device)
		scr.scr_device->dev.init_name = &scr_dev_name[0];

	scr_pinctrl = devm_pinctrl_get(&(scr.scr_device->dev));
	if (IS_ERR_OR_NULL(scr_pinctrl)) {
		printk(KERN_ERR "%s: request pinctrl handle for device failed\n",
		__func__);
		return -EINVAL;
	}

	pctrl_state = pinctrl_lookup_state(scr_pinctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(pctrl_state)) {
		printk(KERN_ERR "%s:pinctrl_lookup_state failed! return %p \n", __func__,
			pctrl_state);
		return -1;
	}

	ret = pinctrl_select_state(scr_pinctrl, pctrl_state);
	if (ret < 0)
		printk(KERN_ERR "%s:pinctrl_select_state failed! return %d \n", __func__,
		ret);

	return ret;
}

static void scr_release_gpio(void)
{
	if (!IS_ERR_OR_NULL(scr_pinctrl))
		devm_pinctrl_put(scr_pinctrl);
	scr_pinctrl = NULL;
}

static unsigned long scr_clk_cfg(void)
{
	unsigned long rate = 0;

	rate = clk_get_rate(scr.scr_clk);
	dprintk(DEBUG_INIT, "%s line %d: scr_clk= 0x%ld\n", __func__, __LINE__, rate);

	if (clk_prepare_enable(scr.scr_clk)) {
		printk(KERN_DEBUG "try to enable scr_clk failed!\n");
	}

	return rate;
}

static void scr_clk_uncfg(void)
{
	if(NULL == scr.scr_clk || IS_ERR(scr.scr_clk)) {
		printk("scr_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(scr.scr_clk);
	}
	return;
}

static uint32_t scr_init_reg(pscr_struct pscr)
{
	unsigned long rate = 0;

	rate = scr_clk_cfg();
	/* rate'unit is hz，card_para.freq'unit is khz */
	pscr->scclk_div = rate/card_para.freq/2000-1;
	pscr->baud_div = (scr.scclk_div + 1)*(card_para.f/card_para.d)-1;

	dprintk(DEBUG_INIT, "%s: scclk_div=%d, baud_div=%d\n", __func__,
		pscr->scclk_div, pscr->baud_div);

	scr_global_interrupt_disable(pscr);

	scr_set_csr_config(pscr, pscr->csr_config);

	scr_clear_interrupt_status(pscr, 0xffffffff);
	scr_set_interrupt_disable(pscr, 0xffffffff);

	scr_flush_txfifo(pscr);
	scr_flush_rxfifo(pscr);

	scr_set_txfifo_threshold(pscr, pscr->txfifo_thh);
	scr_set_rxfifo_threshold(pscr, pscr->rxfifo_thh);

	scr_set_tx_repeat(pscr, pscr->tx_repeat);
	scr_set_rx_repeat(pscr, pscr->rx_repeat);

	scr_set_scclk_divisor(pscr, pscr->scclk_div);
	scr_set_baud_divisor(pscr, pscr->baud_div);
	scr_set_activation_time(pscr, pscr->act_time);
	scr_set_reset_time(pscr, pscr->rst_time);
	scr_set_atrlimit_time(pscr, pscr->atr_time);
	scr_set_guard_time(pscr, pscr->guard_time);
	scr_set_chlimit_time(pscr, pscr->chlimit_time);

	scr_receive_enable(pscr);
	scr_transmit_enable(pscr);
	scr_auto_vpp_enable(pscr);

	scr_set_interrupt_enable(pscr, pscr->inten_bm);
	scr_global_interrupt_enable(&scr);

	/* the default seting is no recv check*/
	if  (1==card_para.recv_no_parity)
		scr_set_recv_parity(pscr, card_para.recv_no_parity&0x01);

	pscr->irq_accsta = 0x00;
	pscr->irq_cursta = 0x00;
	pscr->rxbuf.rptr = 0;
	pscr->rxbuf.wptr = 0;
	pscr->txbuf.rptr = 0;
	pscr->txbuf.wptr = 0;

	pscr->detected = 0;
	pscr->activated = 0;
	pscr->atr_resp = SCR_ATR_RESP_INVALID;

	pscr->chto_flag = 0;

	msleep(1000);

	return 0;
}

static void scr_clear_reg(void)
{
	scr_clear_ctl_reg(&scr);
	return ;
}

static void card_para_default(void)
{
	card_para.f = 372;
	card_para.d = 1;
	card_para.freq = 4000;		              //4Mhz，unit khz
	card_para.recv_no_parity = 1;
}
static void sunxi_scr_params_init(void)
{
	scr.csr_config = 0x0100000;
	/* disable tx irq */
	scr.inten_bm = 0xfffffff0;	              //scr.inten_bm = 0xffffffff;
	scr.txfifo_thh = SCR_FIFO_DEPTH/2;
	scr.rxfifo_thh = SCR_FIFO_DEPTH/4;
	scr.tx_repeat = 0x3;
	scr.rx_repeat = 0x3;
	scr.scclk_div = 0;                            //(APB1CLK/4000000) PCLK/14, <175, && SCCLK >= 1M && =<4M
	scr.baud_div = 0;                             //ETU = 372*SCCLK
	scr.act_time = 1;                             //=1*256, 100;
	scr.rst_time = 0xff;                          //=2*256, >=400
	scr.atr_time = 0xff;                          //scr.atr_time = (40000>>8)+1; //=256*256, 400~40000
	scr.guard_time = 2;                           //=2*ETUs
	scr.chlimit_time = 1024*(10+scr.guard_time);  //1K Characters
}

/**
* 1. the card actually send a lot number data one time, if call the read twice, should copy data according to
*    the length want to read. second read from the last place to start copy.
* 2. timeout seting is 255*20ms = 4.5s
* 3. when write, clear rxbuf
*/
static ssize_t
sunxi_scr_read(struct file *file, char __user *buf,
				size_t size, loff_t *ppos)
{
	int32_t i = 0;
	int32_t rxlen = 0;
	void *from = NULL;
	unsigned long irqflags;
	dprintk(DEBUG_INIT, "%s: enter! size:%ld!\n", __func__, size);

	do {
		spin_lock_irqsave(&rx_lock, irqflags);
		if (!scr_buffer_is_empty(&scr.rxbuf)) {
			rxlen = (scr.rxbuf.wptr - scr.rxbuf.rptr);
			if (rxlen>=size) {
				from = scr.rxbuf.buffer+scr.rxbuf.rptr;
				spin_unlock_irqrestore(&rx_lock, irqflags);
				break;
			}
		}
		spin_unlock_irqrestore(&rx_lock, irqflags);
		dprintk(DEBUG_INIT, "%s: rxlen:%d\n",  __func__, rxlen);
		msleep(20);
		i ++;
	} while (i < 225);

	dprintk(DEBUG_INIT, "%s: rxlen:%d\n",  __func__, rxlen);

	if (size>rxlen)
		size = rxlen;
	if (copy_to_user(buf, from, size)) {
		return -EFAULT;
	}
	scr.rxbuf.rptr += size;

	return size;
}

static ssize_t
sunxi_scr_write( struct file * file, const char __user * buf,
				size_t size, loff_t *ppos )
{
	uint32_t count = size;
	uint8_t data = 0;
	int32_t i = 0;
	unsigned long irqflags;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	if (count > (SCR_BUFFER_SIZE_MASK+1))
		return -EFAULT;
	scr_buffer_flush(&scr.txbuf);
	if (copy_from_user(&(scr.txbuf.buffer), buf, count))
		return -EFAULT;
	else {
		scr.txbuf.wptr = count;
		scr.txbuf.wptr &= (SCR_BUFFER_SIZE_MASK<<1)|0x1;
	}
	scr_display_buffer_data(&(scr.txbuf));
	for (i=0; i<count; ) {
		if (!scr_buffer_is_empty(&scr.txbuf)) {
			if (!scr_txfifo_is_full(&scr)) {
				data = scr_dump_buffer(&scr.txbuf);
				scr_write_fifo(&scr, data);
				i++;
			} else {
				dprintk(DEBUG_INIT, "%s: txfifo full\n", __func__);
				msleep(1);
			}
		} else
			break;
	}
	scr_buffer_flush(&scr.txbuf);
	spin_lock_irqsave(&rx_lock, irqflags);
	scr_buffer_flush(&scr.rxbuf);
	spin_unlock_irqrestore(&rx_lock, irqflags);

	return count;
}

static long sunxi_scr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	/* if SCR_ATR_RESP_FAIL == scr.atr_resp, we must make the received data as atr */
	uint8_t tryTimes = 0;
	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	switch (cmd) {
	case SCR_IOCRESET:
		{
			atr_buffer __user *atreq = argp;
			unsigned long irqflags;
			scr_start_warmreset(&scr);
			while (1) {
				dprintk(DEBUG_DATA_INFO, "%s: atr_resp=%d, activated=%d!!\n",
					__func__, scr.atr_resp, scr.activated);
				if (1 == scr.activated) {
					if (SCR_ATR_RESP_OK == scr.atr_resp) {
						if (copy_to_user(atreq, &(atr_data), sizeof(atr_data)))
							return -EFAULT;
						else
							break;
					} else if (SCR_ATR_RESP_FAIL == scr.atr_resp){
						tryTimes++;
						if (tryTimes==4) {
							spin_lock_irqsave(&rx_lock, irqflags);
							scr_copy_atr_data(&(scr.rxbuf), &atr_data);
							scr_buffer_flush(&scr.rxbuf);
							spin_unlock_irqrestore(&rx_lock, irqflags);
							if (copy_to_user(atreq, &(atr_data), sizeof(atr_data)))
								return -EFAULT;
							else
								break;
						}
					}
				} else {
					scr_start_activation(&scr);
				}
				msleep(10);
			}
		}
		break;
	case SCR_IOCGSTATUS:
		{
			uint32_t detected = scr_detected(&scr);

			dprintk(DEBUG_DATA_INFO, "%s: %d   detected=%d!!\n", __func__,
				__LINE__, detected);
			if (copy_to_user(argp, &(detected),
						 sizeof(detected))) {
					return -EFAULT;
			}
		}
		break;
	case SCR_IOCGPARA:
		{
			cardPara __user *card_Para_eq = argp;

			dprintk(DEBUG_DATA_INFO, "f/d=%d, freq=%d, recv_no_parity=%d!!\n",
				card_para.f, card_para.freq, card_para.recv_no_parity);

			if (copy_to_user(card_Para_eq, &(card_para),
						 sizeof(card_para)))
				return -EFAULT;
		}
		break;
	case SCR_IOCSPARA:
		{
			int temp_data = 0;

			if (copy_from_user(&card_para, argp, sizeof(card_para)))
				return -EFAULT;

			dprintk(DEBUG_DATA_INFO, "d=%d, freq=%d, recv_no_parity=%d!!\n",
				card_para.d, card_para.freq, card_para.recv_no_parity);

			if (0 != card_para.d)
				temp_data = card_para.f/card_para.d;
			else {
				printk(KERN_ERR "card_para ERR d=0!!\n");
				return -1;
			}

			scr.scclk_div = (12000/card_para.freq)-1;
			scr_set_scclk_divisor(&scr, scr.scclk_div);

			scr.baud_div = (scr.scclk_div + 1)*temp_data-1;
			scr_set_baud_divisor(&scr, scr.baud_div);

//			card_para.recv_no_parity &= 0x1;
//			scr_set_recv_parity(&scr, (uint32_t)(card_para.recv_no_parity));
		}
		break;
	default:
		break;
	}
	return 0;
}

static int sunxi_scr_open(struct inode *inode, struct file *file)
{
	int32_t ret = 0;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);
	card_para_default();
	ret = scr_request_gpio();
	if (0 != ret)
		return ret;
	scr_init_reg(&scr);

	return 0;
}

static int sunxi_scr_release(struct inode *inode, struct file *file)
{
	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);
	if (1 == scr.activated)
		scr_start_deactivation(&scr);
	scr_clear_reg();
	scr_clk_uncfg();
	scr_release_gpio();

	return 0;
}

static const struct file_operations sunxi_scr_fops = {
	.owner = THIS_MODULE,
	.llseek = noop_llseek,
	.read = sunxi_scr_read,
	.write = sunxi_scr_write,
	.unlocked_ioctl = sunxi_scr_ioctl,
	.open = sunxi_scr_open,
	.release = sunxi_scr_release,
};

static int sunxi_scr_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct resource *mem_res = NULL;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	scr.scr_device = pdev;
	if (!of_device_is_available(node)) {
		printk("%s: smartcard status disable!!\n", __func__);
		return -EPERM;
	}

	scr.irq_no = platform_get_irq(pdev, 0);
	if (scr.irq_no < 0) {
		printk(KERN_ERR "%s:get irq number failed!\n",  __func__);
		return -EINVAL;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem_res == NULL) {
		printk(KERN_ERR "%s: failed to get MEM res\n", __func__);
		return -ENXIO;
	}

	if (!request_mem_region(mem_res->start, resource_size(mem_res), mem_res->name)) {
		printk(KERN_ERR  "%s: failed to request mem region\n", __func__);
		return -EINVAL;
	}

	scr.reg_base = ioremap(mem_res->start, resource_size(mem_res));
	if (!scr.reg_base) {
		printk(KERN_ERR  "%s: failed to io remap\n", __func__);
		return -EIO;
	}

	scr.scr_clk = of_clk_get(node, 0);
	if (!scr.scr_clk || IS_ERR(scr.scr_clk)) {
		printk(KERN_ERR "try to get scr clock failed!\n");
		return -EINVAL;
	}

	scr.scr_clk_source = of_clk_get(node, 1);
	if(!scr.scr_clk_source || IS_ERR(scr.scr_clk_source)) {
		printk("%s err: try to get scr_clk_source clock failed!\n", __func__);
		return -EINVAL;
	}

	if (of_property_read_u32(node, "clock-frequency", &scr.clk_freq))
		scr.clk_freq = 24000000;

	if(clk_set_parent(scr.scr_clk, scr.scr_clk_source))
		printk("%s: set scr_clk parent to scr_clk_source failed!\n", __func__);

	if (clk_set_rate(scr.scr_clk, scr.clk_freq)) {
		printk(KERN_DEBUG "set ir scr_clk freq to 24M failed!\n");
	}

	sunxi_scr_params_init();

	scr_wq = create_singlethread_workqueue("scr_wq");
	if (!scr_wq) {
		printk(KERN_ERR "Creat scr workqueue failed.\n");
		return -ENOMEM;
	}

	INIT_WORK(&scr_work, scr_work_func);

	if (request_irq(scr.irq_no, scr_irq_service, 0, "scr", NULL)) {
		return -1;
	}

	if (sunxi_scr_major == -1) {
		if ((sunxi_scr_major = register_chrdev (0, SCR_MODULE_NAME, &sunxi_scr_fops)) < 0) {
			printk(KERN_ERR "%s: Failed to register character device\n", __func__);
			return -1;
		} else
			dprintk(DEBUG_INIT, "%s: sunxi_scr_major = %d\n", __func__, sunxi_scr_major);
	}

	scr_dev_class = class_create(THIS_MODULE, SCR_MODULE_NAME);
	if (IS_ERR(scr_dev_class))
		return -1;
	scr_device = device_create(scr_dev_class, NULL,  MKDEV(sunxi_scr_major, 0), NULL, SCR_MODULE_NAME);

	return 0;

}

static int sunxi_scr_remove(struct platform_device *pdev)
{
	if (sunxi_scr_major > 0) {
		device_destroy(scr_dev_class, MKDEV(sunxi_scr_major, 0));
		class_destroy(scr_dev_class);
		unregister_chrdev(sunxi_scr_major, SCR_MODULE_NAME);
	}
	free_irq(scr.irq_no, NULL);
	cancel_work_sync(&scr_work);
	if (NULL != scr_wq) {
		flush_workqueue(scr_wq);
		destroy_workqueue(scr_wq);
		scr_wq = NULL;
	}

	if(NULL == scr.scr_clk || IS_ERR(scr.scr_clk)) {
		printk("scr_clk handle is invalid, just return!\n");
		return -1;
	} else {
		clk_disable_unprepare(scr.scr_clk);
		clk_put(scr.scr_clk);
		scr.scr_clk = NULL;
	}
	if(NULL == scr.scr_clk_source || IS_ERR(scr.scr_clk_source)) {
		printk("scr_clk_source handle is invalid, just return!\n");
		return -1;
	} else {
		clk_put(scr.scr_clk_source);
		scr.scr_clk_source = NULL;
	}

	printk(KERN_INFO "%s: module unloaded\n", __func__);

	return 0;
}

static const struct of_device_id sunxi_scr_match[] = {
	{ .compatible = "allwinner,sunxi-scr", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_scr_match);


static struct platform_driver scr_platform_driver = {
	.probe  = sunxi_scr_probe,
	.remove = sunxi_scr_remove,
	.driver = {
		.name	 = SCR_MODULE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_scr_match,
	},
};

static int __init sunxi_scr_init(void)
{
	return platform_driver_register(&scr_platform_driver);
}

static void __exit sunxi_scr_exit(void)
{
	platform_driver_unregister(&scr_platform_driver);
}

module_init(sunxi_scr_init);
module_exit(sunxi_scr_exit);
module_param_named(scr_debug_mask, scr_debug_mask, int, 0644);
MODULE_DESCRIPTION("Smart Card driver");
MODULE_AUTHOR("Ming Li");
MODULE_LICENSE("GPL");

