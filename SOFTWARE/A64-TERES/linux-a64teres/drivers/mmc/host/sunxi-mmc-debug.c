#include <linux/clk.h>
#include <linux/clk/sunxi.h>

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/reset.h>

#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/stat.h>


#include <linux/mmc/host.h>
#include "sunxi-mmc.h"
#include "sunxi-mmc-debug.h"
#include "sunxi-mmc-export.h"
#include "sunxi-mmc-sun50iw1p1-2.h"


#define GPIO_BASE_ADDR	0x1c20800
#define CCMU_BASE_ADDR	0x1c20000

static struct device_attribute dump_register[3];


void sunxi_mmc_dumphex32(struct sunxi_mmc_host* host, char* name, char* base, int len)
{
	u32 i;
	printk("dump %s registers:", name);
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
			printk("\n0x%p : ", base + i);
		printk("0x%08x ",__raw_readl(host->reg_base+i));
	}
	printk("\n");
}

void sunxi_mmc_dump_des(struct sunxi_mmc_host* host, char* base, int len)
{
	u32 i;
	printk("dump des mem\n");
	for (i=0; i<len; i+=4) {
		if (!(i&0xf))
			printk("\n0x%p : ", base + i);
		printk("0x%08x ",*(u32 *)(base+i));
	}
	printk("\n");
}



static ssize_t maual_insert_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	int ret;

	ret = snprintf(buf, PAGE_SIZE, "Usage: \"echo 1 > insert\" to scan card\n");
	return ret;
}

static ssize_t maual_insert_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int ret;
	char *end;
	unsigned long insert = simple_strtoul(buf, &end, 0);
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	if (end == buf) {
		ret = -EINVAL;
		return ret;
	}

	dev_info(dev, "insert %ld\n", insert);
	if(insert)
		mmc_detect_change(mmc,0);
	else
		dev_info(dev, "no detect change\n");
	//sunxi_mmc_rescan_card(0);
	
	ret = count;
	return ret;
}


void dump_reg(struct sunxi_mmc_host *host)
{
	int i = 0;
	void __iomem *gpio_ptr =  ioremap(GPIO_BASE_ADDR, 0x300);
	void __iomem *ccmu_ptr =  ioremap(CCMU_BASE_ADDR, 0x400);


	printk("Dump sdmmc regs:\n");
	for (i=0; i<0x180; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(host->reg_base + i));
		printk("%08x ", readl(host->reg_base + i));
	}
	printk("\n");


	printk("Dump gpio regs:\n");
	
	for (i=0; i<0x120; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(gpio_ptr + i));
		printk("%08x ", readl(gpio_ptr + i));
	}
	printk("\n");

	printk("Dump gpio irqc regs:\n");
	for (i=0x200; i<0x260; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(gpio_ptr + i));
		printk("%08x ", readl(gpio_ptr + i));
	}
	printk("\n");


	printk("Dump ccmu regs:gating\n");
	for (i=0x60; i<=0x80; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(ccmu_ptr + i));
		printk("%08x ", readl(ccmu_ptr + i));
	}
	printk("\n");


	printk("Dump ccmu regs:module clk\n");
	for (i=0x80; i<=0x100; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(ccmu_ptr + i));
		printk("%08x ", readl(ccmu_ptr + i));
	}
	printk("\n");

	printk("Dump ccmu regs:reset\n");
	for (i=0x2c0; i<=0x2e0; i+=4) {
		if (!(i&0xf))
			printk("\n0x%08llx : ", (u64)(ccmu_ptr + i));
		printk("%08x ", readl(ccmu_ptr + i));
	}
	printk("\n");

	iounmap(gpio_ptr);
	iounmap(ccmu_ptr);

}


/*
static ssize_t dump_register_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	char *p = buf;
	int i = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	struct sunxi_mmc_host *host = mmc_priv(mmc);
	void __iomem *gpio_ptr =  ioremap(GPIO_BASE_ADDR, 0x300);
	void __iomem *ccmu_ptr =  ioremap(CCMU_BASE_ADDR, 0x400);


	p += sprintf(p, "Dump sdmmc regs:\n");
	for (i=0; i<0x180; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(host->reg_base + i));
		p += sprintf(p, "%08x ", readl(host->reg_base + i));
	}
	p += sprintf(p, "\n");


	p += sprintf(p, "Dump gpio regs:\n");
	
	for (i=0; i<0x120; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(gpio_ptr + i));
		p += sprintf(p, "%08x ", readl(gpio_ptr + i));
	}
	p += sprintf(p, "\n");

	p += sprintf(p, "Dump gpio irqc regs:\n");
	for (i=0x200; i<0x260; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(gpio_ptr + i));
		p += sprintf(p, "%08x ", readl(gpio_ptr + i));
	}
	p += sprintf(p, "\n");


	p += sprintf(p, "Dump ccmu regs:gating\n");
	for (i=0x60; i<=0x80; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(ccmu_ptr + i));
		p += sprintf(p, "%08x ", readl(ccmu_ptr + i));
	}
	p += sprintf(p, "\n");


	p += sprintf(p, "Dump ccmu regs:module clk\n");
	for (i=0x80; i<=0x100; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(ccmu_ptr + i));
		p += sprintf(p, "%08x ", readl(ccmu_ptr + i));
	}
	p += sprintf(p, "\n");

	p += sprintf(p, "Dump ccmu regs:reset\n");
	for (i=0x2c0; i<=0x2e0; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(ccmu_ptr + i));
		p += sprintf(p, "%08x ", readl(ccmu_ptr + i));
	}
	p += sprintf(p, "\n");


	iounmap(gpio_ptr);
	iounmap(ccmu_ptr);

	//dump_reg(host);

	return p-buf;

}
*/


static ssize_t dump_host_reg_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	char *p = buf;
	int i = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	struct sunxi_mmc_host *host = mmc_priv(mmc);

	p += sprintf(p, "Dump sdmmc regs:\n");
	for (i=0; i<0x180; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(host->reg_base + i));
		p += sprintf(p, "%08x ", readl(host->reg_base + i));
	}
	p += sprintf(p, "\n");

	return p-buf;

}



static ssize_t dump_gpio_reg_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	char *p = buf;
	int i = 0;
	void __iomem *gpio_ptr =  ioremap(GPIO_BASE_ADDR, 0x300);

	p += sprintf(p, "Dump gpio regs:\n");
	for (i=0; i<0x120; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(gpio_ptr + i));
		p += sprintf(p, "%08x ", readl(gpio_ptr + i));
	}
	p += sprintf(p, "\n");

	p += sprintf(p, "Dump gpio irqc regs:\n");
	for (i=0x200; i<0x260; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(gpio_ptr + i));
		p += sprintf(p, "%08x ", readl(gpio_ptr + i));
	}
	p += sprintf(p, "\n");

	iounmap(gpio_ptr);

	return p-buf;

}



static ssize_t dump_ccmu_reg_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	char *p = buf;
	int i = 0;
	void __iomem *ccmu_ptr =  ioremap(CCMU_BASE_ADDR, 0x400);


	p += sprintf(p, "Dump ccmu\n");
	for (i=0x0; i<=0x400; i+=4) {
		if (!(i&0xf))
			p += sprintf(p, "\n0x%08llx : ", (u64)(ccmu_ptr + i));
		p += sprintf(p, "%08x ", readl(ccmu_ptr + i));
	}
	p += sprintf(p, "\n");

	iounmap(ccmu_ptr);
	//dump_reg(host);
	return p-buf;

}



static ssize_t dump_clk_dly_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	char *p = buf;
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	struct sunxi_mmc_host *host = mmc_priv(mmc);

	if(host->sunxi_mmc_dump_dly_table){
		host->sunxi_mmc_dump_dly_table(host);
	}else{
		dev_warn(mmc_dev(mmc),"not found the dump dly table\n");
	}

	return p-buf;
}



int mmc_create_sys_fs(struct sunxi_mmc_host* host,struct platform_device *pdev)
{
	int ret;

	host->maual_insert.show = maual_insert_show;
	host->maual_insert.store = maual_insert_store;
	sysfs_attr_init(host->maual_insert.attr);
	host->maual_insert.attr.name = "sunxi_insert";
	host->maual_insert.attr.mode = S_IRUGO | S_IWUSR;
	ret = device_create_file(&pdev->dev, &host->maual_insert);
	if(ret)
		return ret;
	
	host->dump_register = dump_register;
	host->dump_register[0].show = dump_host_reg_show;
	sysfs_attr_init(host->dump_register[0].attr);
	host->dump_register[0].attr.name = "sunxi_dump_host_regster";
	host->dump_register[0].attr.mode = S_IRUGO;
	ret = device_create_file(&pdev->dev, &host->dump_register[0]);
	if(ret)
		return ret;

	host->dump_register[1].show = dump_gpio_reg_show;
	sysfs_attr_init(host->dump_register[1].attr);
	host->dump_register[1].attr.name = "sunxi_dump_gpio_regster";
	host->dump_register[1].attr.mode = S_IRUGO;
	ret = device_create_file(&pdev->dev, &host->dump_register[1]);
	if(ret)
		return ret;


	host->dump_register[2].show = dump_ccmu_reg_show;
	sysfs_attr_init(host->dump_register[2].attr);
	host->dump_register[2].attr.name = "sunxi_dump_ccmu_regster";
	host->dump_register[2].attr.mode = S_IRUGO;
	ret = device_create_file(&pdev->dev, &host->dump_register[2]);
	if(ret)
		return ret;


	host->dump_clk_dly.show = dump_clk_dly_show;
	sysfs_attr_init(host->dump_clk_dly.attr);
	host->dump_clk_dly.attr.name = "sunxi_dump_clk_dly";
	host->dump_clk_dly.attr.mode = S_IRUGO;
	ret = device_create_file(&pdev->dev, &host->dump_clk_dly);
	if(ret)
		return ret;

	
	return ret;
}

void mmc_remove_sys_fs(struct sunxi_mmc_host* host,struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &host->maual_insert);
	device_remove_file(&pdev->dev, &host->dump_register[0]);
	device_remove_file(&pdev->dev, &host->dump_register[1]);
	device_remove_file(&pdev->dev, &host->dump_register[2]);
	device_remove_file(&pdev->dev, &host->dump_clk_dly);
}



