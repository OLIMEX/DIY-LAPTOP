/* Copyright (C) 2014 ALLWINNERTECH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <media/rc-core.h>
#include "sunxi-ir-rx.h"

#define SUNXI_IR_DRIVER_NAME	"sunxi-rc-recv"
#define SUNXI_IR_DEVICE_NAME	"sunxi_ir_recv"

DEFINE_IR_RAW_EVENT(rawir);
static struct sunxi_ir_data *ir_data;
static struct rc_dev *sunxi_rcdev;
static u32 is_receiving = 0;
static bool pluse_pre = 0;
static char ir_dev_name[] = "s_cir_rx";

static int debug_mask = 0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(fmt , ## arg)
#define IR_BASE 			(ir_data->reg_base)

static inline u8 ir_get_data(void)
{
	return (u8)(readl(IR_BASE + IR_RXDAT_REG));
}

static inline u32 ir_get_intsta(void)
{
	return (readl(IR_BASE + IR_RXINTS_REG));
}

static inline void ir_clr_intsta(u32 bitmap)
{
	u32 tmp = readl(IR_BASE + IR_RXINTS_REG);

	tmp &= ~0xff;
	tmp |= bitmap&0xff;
	writel(tmp, IR_BASE + IR_RXINTS_REG);
}

#ifdef CONFIG_OF
/* Translate OpenFirmware node properties into platform_data */
static struct of_device_id sunxi_ir_recv_of_match[] = {
	{ .compatible = "allwinner,s_cir", },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_ir_recv_of_match);
#else /* !CONFIG_OF */
#endif

static irqreturn_t sunxi_ir_recv_irq(int irq, void *dev_id)
{
	u32 intsta,dcnt;
	u32 i = 0;
	bool pluse_now = 0;
	u8 reg_data;

	dprintk(DEBUG_INT, "IR RX IRQ Serve\n");

	intsta = ir_get_intsta();
	ir_clr_intsta(intsta);

	/* get ther count of signal */
	dcnt =  (intsta>>8) & 0x7f;
	dprintk(DEBUG_INT, "receive cnt :%d \n", dcnt);
	
	/* Read FIFO and fill the raw event */
	for (i=0; i<dcnt; i++) {
		/* get the data from fifo */
		reg_data = ir_get_data();
		pluse_now = (reg_data & 0x80)? true : false;
			
		if( pluse_pre == pluse_now){/* the signal maintian */
			/* the pluse or space lasting*/
			rawir.duration += (u32)(reg_data & 0x7f);
			dprintk(DEBUG_ERR,"raw: %d:%d \n",(reg_data & 0x80)>>7,(reg_data & 0x7f));
		}else{
			if(is_receiving){
				rawir.duration *= IR_SIMPLE_UNIT;
				dprintk(DEBUG_INT,"pusle :%d, dur: %u ns\n",rawir.pulse,rawir.duration );
				ir_raw_event_store(sunxi_rcdev, &rawir);
				rawir.pulse = pluse_now;
				rawir.duration = (u32)(reg_data & 0x7f);	
				dprintk(DEBUG_ERR,"raw: %d:%d \n",(reg_data & 0x80)>>7,(reg_data & 0x7f));
			}else{
				/* get the first pluse signal */
				rawir.pulse = pluse_now;
				rawir.duration = (u32)(reg_data & 0x7f);
				#ifdef CIR_FPGA
				rawir.duration += ((IR_ACTIVE_T>>16)+1) * ((IR_ACTIVE_T_C>>23 )? 128:1);
				dprintk(DEBUG_INT, "get frist pulse,add head %d !!\n",((IR_ACTIVE_T>>16)+1) * ((IR_ACTIVE_T_C>>23 )? 128:1));
				#endif
				is_receiving = 1;
				dprintk(DEBUG_ERR,"raw: %d:%d \n",(reg_data & 0x80)>>7,(reg_data & 0x7f));
			}
			pluse_pre = pluse_now;
		}	
	}
	
	if (intsta & IR_RXINTS_RXPE){
		if(rawir.duration){
			rawir.duration *= IR_SIMPLE_UNIT;
			dprintk(DEBUG_INT,"pusle :%d, dur: %u ns\n",rawir.pulse,rawir.duration );
			ir_raw_event_store(sunxi_rcdev, &rawir);
		}
		dprintk(DEBUG_INT, "handle raw data.\n");
		/* handle ther decoder theread */
		ir_raw_event_handle(sunxi_rcdev);
		is_receiving = 0;
		pluse_pre = false;
	}

	if (intsta & IR_RXINTS_RXOF) {
		/* FIFO Overflow */
		pr_err("ir_rx_irq_service: Rx FIFO Overflow!!\n");
		is_receiving = 0;
		pluse_pre = false;
	}

	return IRQ_HANDLED;
}

static void ir_mode_set(enum ir_mode set_mode)
{
	u32 ctrl_reg = 0;

	switch (set_mode) {
	case CIR_MODE_ENABLE:
		ctrl_reg = readl(IR_BASE+IR_CTRL_REG);
		ctrl_reg |= IR_CIR_MODE;
		break;
	case IR_MODULE_ENABLE:
		ctrl_reg = readl(IR_BASE+IR_CTRL_REG);
		ctrl_reg |= IR_ENTIRE_ENABLE;
		break;
	default:
		return;
	}
	writel(ctrl_reg, IR_BASE+IR_CTRL_REG);
}

static void ir_sample_config(enum ir_sample_config set_sample)
{
	u32 sample_reg = 0;

	sample_reg = readl(IR_BASE+IR_SPLCFG_REG);

	switch (set_sample) {
	case IR_SAMPLE_REG_CLEAR:
		sample_reg = 0;
		break;
	case IR_CLK_SAMPLE:
		sample_reg |= IR_SAMPLE_DEV;
		break;
	case IR_FILTER_TH:
		sample_reg |= IR_RXFILT_VAL;
		break;
	case IR_IDLE_TH:
		sample_reg |= IR_RXIDLE_VAL;
		break;
	case IR_ACTIVE_TH:
		sample_reg |= IR_ACTIVE_T;
		sample_reg |= IR_ACTIVE_T_C;
		break;
	default:
		return;
	}
	writel(sample_reg, IR_BASE+IR_SPLCFG_REG);
}

static void ir_signal_invert(void)
{
	u32 reg_value;
	reg_value = 0x1<<2;
	writel(reg_value, IR_BASE+IR_RXCFG_REG);
}

static void ir_irq_config(enum ir_irq_config set_irq)
{
	u32 irq_reg = 0;

	switch (set_irq) {
	case IR_IRQ_STATUS_CLEAR:
		writel(0xef, IR_BASE+IR_RXINTS_REG);
		return;
	case IR_IRQ_ENABLE:
		irq_reg = readl(IR_BASE+IR_RXINTE_REG);
		irq_reg |= IR_IRQ_STATUS;
		break;
	case IR_IRQ_FIFO_SIZE:
		irq_reg = readl(IR_BASE+IR_RXINTE_REG);
		irq_reg |= IR_FIFO_32;
		break;
	default:
		return;
	}
	writel(irq_reg, IR_BASE+IR_RXINTE_REG);
}

static void ir_reg_cfg(void)
{
	/* Enable IR Mode */
	ir_mode_set(CIR_MODE_ENABLE);
	/* Config IR Smaple Register */
	ir_sample_config(IR_SAMPLE_REG_CLEAR);
	ir_sample_config(IR_CLK_SAMPLE);
	ir_sample_config(IR_FILTER_TH);		/* Set Filter Threshold */
	ir_sample_config(IR_IDLE_TH); 		/* Set Idle Threshold */
	ir_sample_config(IR_ACTIVE_TH);         /* Set Active Threshold */
	/* Invert Input Signal */
	ir_signal_invert();
	/* Clear All Rx Interrupt Status */
	ir_irq_config(IR_IRQ_STATUS_CLEAR);
	/* Set Rx Interrupt Enable */
	ir_irq_config(IR_IRQ_ENABLE);
	ir_irq_config(IR_IRQ_FIFO_SIZE);	/* Rx FIFO Threshold = FIFOsz/2; */
	/* Enable IR Module */
	ir_mode_set(IR_MODULE_ENABLE);

	return;
}

static void ir_clk_cfg(void)
{

	unsigned long rate = 0;

	rate = clk_get_rate(ir_data->pclk);
	dprintk(DEBUG_INIT, "%s: get ir_clk_source rate %dHZ\n", __func__, (__u32)rate);

	if(clk_set_parent(ir_data->mclk, ir_data->pclk))
		pr_err("%s: set ir_clk parent to ir_clk_source failed!\n", __func__);

	if (clk_set_rate(ir_data->mclk, IR_CLK)) {
		pr_err("set ir clock freq to 4M failed!\n");
	}
	rate = clk_get_rate(ir_data->mclk);
	dprintk(DEBUG_INIT, "%s: get ir_clk rate %dHZ\n", __func__, (__u32)rate);

	if (clk_prepare_enable(ir_data->mclk)) {
			pr_err("try to enable ir_clk failed!\n");
	}

	return;
}

static void ir_clk_uncfg(void)
{

	if(NULL == ir_data->mclk || IS_ERR(ir_data->mclk)) {
		pr_err("ir_clk handle is invalid, just return!\n");
		return;
	} else {
		clk_disable_unprepare(ir_data->mclk);
		clk_put(ir_data->mclk);
		ir_data->mclk = NULL;
	}

	if(NULL == ir_data->pclk || IS_ERR(ir_data->pclk)) {
		pr_err("ir_clk_source handle is invalid, just return!\n");
		return;
	} else {
		clk_put(ir_data->pclk);
		ir_data->pclk = NULL;
	}
	return;
}

static void ir_setup(void)
{
	dprintk(DEBUG_INIT, "ir_rx_setup: ir setup start!!\n");

	ir_clk_cfg();
	ir_reg_cfg();

	dprintk(DEBUG_INIT, "ir_rx_setup: ir setup end!!\n");
	return;
}

static int sunxi_ir_startup(struct platform_device *pdev)
{
	struct device_node *np =NULL;
	int ret = 0;
	const char *name = NULL;
	
	ir_data = kzalloc(sizeof(*ir_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(ir_data)) {
		pr_err("ir_data: not enough memory for ir data\n");
		return -ENOMEM;
	}

	np = pdev->dev.of_node;
	
	ir_data->reg_base= of_iomap(np, 0);
	if (NULL == ir_data->reg_base) {
		pr_err("%s:Failed to ioremap() io memory region.\n",__func__);
		ret = -EBUSY;
	}else
		dprintk(DEBUG_INIT, "ir base: %p !\n",ir_data->reg_base);
	ir_data->irq_num= irq_of_parse_and_map(np, 0);
	if (0 == ir_data->irq_num) {
		pr_err("%s:Failed to map irq.\n", __func__);
		ret = -EBUSY;
	}else
		dprintk(DEBUG_INIT, "ir irq num: %d !\n",ir_data->irq_num);
	ir_data->pclk = of_clk_get(np, 0);
	ir_data->mclk = of_clk_get(np, 1);
	if (NULL==ir_data->pclk||IS_ERR(ir_data->pclk)
		||NULL==ir_data->mclk||IS_ERR(ir_data->mclk)) {
		pr_err("%s:Failed to get clk.\n", __func__);
		ret = -EBUSY;
	}
	if (of_property_read_u32(np, "ir_addr_code", &ir_data->ir_addr)) {
		pr_err("%s: get cir addr failed", __func__);
		ret =  -EBUSY;
	}
	if (of_property_read_string(np, "supply", &name)) {
		pr_err("%s: cir have no power supply\n", __func__);
		ir_data->suply = NULL;
	}else{
		ir_data->suply = regulator_get(NULL, name);
		if(IS_ERR(ir_data->pclk)){
			pr_err("%s: cir get supply err\n", __func__);
			ir_data->suply = NULL;
		}
	}

	return ret;
}

static int sunxi_ir_recv_probe(struct platform_device *pdev)
{
	int rc;

	dprintk(DEBUG_INIT, "sunxi-ir probe start !\n");

	if (pdev->dev.of_node) {
		/* get dt and sysconfig */
		rc = sunxi_ir_startup(pdev);
	}else{
		pr_err("sunxi ir device tree err!\n");
		return -EBUSY;
	}

	if( rc < 0)
		goto err_allocate_device;

	sunxi_rcdev = rc_allocate_device();
	if (!sunxi_rcdev) {
		rc = -ENOMEM;
		pr_err("rc dev allocate fail !\n");
		goto err_allocate_device;
	}

	sunxi_rcdev->driver_type = RC_DRIVER_IR_RAW;
	sunxi_rcdev->input_name = SUNXI_IR_DEVICE_NAME;
	sunxi_rcdev->input_phys = SUNXI_IR_DEVICE_NAME "/input0";
	sunxi_rcdev->input_id.bustype = BUS_HOST;
	sunxi_rcdev->input_id.vendor = 0x0001;
	sunxi_rcdev->input_id.product = 0x0001;
	sunxi_rcdev->input_id.version = 0x0100;
	sunxi_rcdev->dev.parent = &pdev->dev;
	sunxi_rcdev->driver_name = SUNXI_IR_DRIVER_NAME;

	sunxi_rcdev->allowed_protos = (u64)RC_BIT_NEC;
	sunxi_rcdev->map_name = RC_MAP_SUNXI;

	init_rc_map_sunxi(ir_data->ir_addr);
	rc = rc_register_device(sunxi_rcdev);
	if (rc < 0) {
		dev_err(&pdev->dev, "failed to register rc device\n");
		goto err_register_rc_device;
	}
	sunxi_rcdev->enabled_protocols = sunxi_rcdev->allowed_protos;;
	sunxi_rcdev->input_dev->dev.init_name = &ir_dev_name[0];

	if (0 != rc) {
		pr_err("%s: config ir rx pin err.\n", __func__);
		goto err_platfrom_device;
	}

	platform_set_drvdata(pdev, sunxi_rcdev);
	ir_data->rcdev = sunxi_rcdev;
	if(ir_data->suply)
		rc = regulator_enable(ir_data->suply);
	ir_setup();
	
	if (request_irq(ir_data->irq_num, sunxi_ir_recv_irq, IRQF_DISABLED, "RemoteIR_RX",
			sunxi_rcdev)) {
		pr_err("%s: request irq fail.\n", __func__);
		rc = -EBUSY;
		goto err_request_irq;
	}

	/* enable here */
	dprintk(DEBUG_INIT, "ir probe end!\n");
	return 0;

err_request_irq:
	platform_set_drvdata(pdev, NULL);
	rc_unregister_device(sunxi_rcdev);
	sunxi_rcdev = NULL;
	ir_clk_uncfg();
	if(ir_data->suply){
		regulator_disable(ir_data->suply);
		regulator_put(ir_data->suply);
	}
err_platfrom_device:
	exit_rc_map_sunxi();
err_register_rc_device:
	rc_free_device(sunxi_rcdev);
err_allocate_device:
	if(ir_data)
		kfree(ir_data);
	return rc;
}

static int sunxi_ir_recv_remove(struct platform_device *pdev)
{
	free_irq(ir_data->irq_num, sunxi_rcdev);
	ir_clk_uncfg();
	platform_set_drvdata(pdev, NULL);
	if(ir_data->suply){
		regulator_disable(ir_data->suply);
		regulator_put(ir_data->suply);
	}
	rc_unregister_device(sunxi_rcdev);
	exit_rc_map_sunxi();
	if(ir_data)
		kfree(ir_data);
	return 0;
}

#ifdef CONFIG_PM
static int sunxi_ir_recv_suspend(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "enter: sunxi_ir_rx_suspend. \n");

	disable_irq_nosync(ir_data->irq_num);

	if(NULL == ir_data->mclk || IS_ERR(ir_data->mclk)) {
		pr_err("ir_clk handle is invalid, just return!\n");
		return -1;
	} else {
		clk_disable_unprepare(ir_data->mclk);
	}
	return 0;
}

static int sunxi_ir_recv_resume(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "enter: sunxi_ir_rx_resume. \n");

	clk_prepare_enable(ir_data->mclk);
	ir_reg_cfg();
	enable_irq(ir_data->irq_num);

	return 0;
}

static const struct dev_pm_ops sunxi_ir_recv_pm_ops = {
	.suspend        = sunxi_ir_recv_suspend,
	.resume         = sunxi_ir_recv_resume,
};
#endif

static struct platform_driver sunxi_ir_recv_driver = {
	.probe  = sunxi_ir_recv_probe,
	.remove = sunxi_ir_recv_remove,
	.driver = {
		.name   = SUNXI_IR_DRIVER_NAME,
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(sunxi_ir_recv_of_match),
#ifdef CONFIG_PM
		.pm	= &sunxi_ir_recv_pm_ops,
#endif
	},
};
module_platform_driver(sunxi_ir_recv_driver);
module_param_named(debug_mask, debug_mask, int, 0644);
MODULE_DESCRIPTION("SUNXI IR Receiver driver");
MODULE_AUTHOR("QIn");
MODULE_LICENSE("GPL v2");
