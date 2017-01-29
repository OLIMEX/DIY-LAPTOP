#include "axp-rw.h"
#include "axp-cfg.h"

static u8 axp_reg_addr = 0;

#ifndef RSB_RTSADDR_AXP809
#define  RSB_RTSADDR_AXP809 (0x2d)
#endif
#ifndef RSB_RTSADDR_AXP806
#define  RSB_RTSADDR_AXP806 (0x3a)
#endif

static void axp81x_mfd_irq_work(struct work_struct *work)
{
	struct axp_dev *chip = container_of(work, struct axp_dev, irq_work);
	u64 irqs = 0;

	while (1) {
		if (chip->ops->read_irqs(chip, &irqs)) {
			printk("read irq fail\n");
			break;
		}
		irqs &= chip->irqs_enabled;
		if (irqs == 0) {
			break;
		 }

		if(irqs > 0xffffffff) {
			blocking_notifier_call_chain(&chip->notifier_list, (u32)(irqs>>32), (void *)1);
		} else {
			blocking_notifier_call_chain(&chip->notifier_list, (u32)irqs, (void *)0);
		}
	}
#ifdef	CONFIG_AXP_TWI_USED
	enable_irq(chip->client->irq);
#elif defined(CONFIG_AXP_NMI_USED)
	enable_nmi();
#endif
	return;
}

static s32 axp81x_disable_irqs(struct axp_dev *chip, u64 irqs)
{
	u8 v[11];
	s32 ret;
	unsigned char devaddr = RSB_RTSADDR_AXP809;

	chip->irqs_enabled &= ~irqs;

	v[0] = ((chip->irqs_enabled) & 0xff);
	v[1] = AXP81X_INTEN2;
	v[2] = ((chip->irqs_enabled) >> 8) & 0xff;
	v[3] = AXP81X_INTEN3;
	v[4] = ((chip->irqs_enabled) >> 16) & 0xff;
	v[5] = AXP81X_INTEN4;
	v[6] = ((chip->irqs_enabled) >> 24) & 0xff;
	v[7] = AXP81X_INTEN5;
	v[8] = ((chip->irqs_enabled) >> 32) & 0xff;
	v[9] = AXP81X_INTEN6;
	v[10] = ((chip->irqs_enabled) >> 32) & 0xff;
	ret =  __axp_writes(&devaddr, chip->client, AXP81X_INTEN1, 11, v, false);

	return ret;
}

static s32 axp81x_enable_irqs(struct axp_dev *chip, u64 irqs)
{
	u8 v[11];
	s32 ret;
	unsigned char devaddr = RSB_RTSADDR_AXP809;

	chip->irqs_enabled |=  irqs;

	v[0] = ((chip->irqs_enabled) & 0xff);
	v[1] = AXP81X_INTEN2;
	v[2] = ((chip->irqs_enabled) >> 8) & 0xff;
	v[3] = AXP81X_INTEN3;
	v[4] = ((chip->irqs_enabled) >> 16) & 0xff;
	v[5] = AXP81X_INTEN4;
	v[6] = ((chip->irqs_enabled) >> 24) & 0xff;
	v[7] = AXP81X_INTEN5;
	v[8] = ((chip->irqs_enabled) >> 32) & 0xff;
	v[9] = AXP81X_INTEN6;
	v[10] = ((chip->irqs_enabled) >> 32) & 0xff;
	ret =  __axp_writes(&devaddr, chip->client, AXP81X_INTEN1, 11, v, false);

	return ret;
}

static s32 axp81x_read_irqs(struct axp_dev *chip, u64 *irqs)
{
	u8 v[6] = {0, 0, 0, 0, 0, 0};
	s32 ret;
	unsigned char devaddr = RSB_RTSADDR_AXP809;
	ret =  __axp_reads(&devaddr, chip->client, AXP81X_INTSTS1, 6, v, false);
	if (ret < 0)
		return ret;

	*irqs = (((u64) v[5]) << 40) | (((u64) v[4]) << 32) |(((u64) v[3]) << 24) |
		(((u64) v[2])<< 16) | (((u64)v[1]) << 8) | ((u64) v[0]);
	return 0;
}

static s32  axp81x_init_chip(struct axp_dev *chip)
{
	u8 chip_id = 0;
	u8 v[23] = {0xd8,AXP81X_INTEN2, 0xfc,AXP81X_INTEN3,0x00,
			  AXP81X_INTEN4, 0x01,AXP81X_INTEN5, 0x18,
			  AXP81X_INTEN6, 0x00,AXP81X_INTSTS1,0xff,
			  AXP81X_INTSTS2, 0xff,AXP81X_INTSTS3,0xff,
			  AXP81X_INTSTS4, 0xff,AXP81X_INTSTS5,0xff,
			  AXP81X_INTSTS6,0xff};
	unsigned char devaddr = RSB_RTSADDR_AXP809;
	s32 err;

	INIT_WORK(&chip->irq_work, axp81x_mfd_irq_work);

	/*read chip id*/	//???which int should enable must check with SD4
	err =  __axp_read(&devaddr, chip->client, AXP81X_IC_TYPE, &chip_id, false);
	if (err) {
		printk("[AXP81X-MFD] try to read chip id failed!\n");
		return err;
	}

	if((chip_id & 0xff) == 0x51)
		chip->type = AXP81X;
	else
		chip->type = 0;

	/*enable irqs and clear*/
	err =  __axp_writes(&devaddr, chip->client, AXP81X_INTEN1, 23, v, false);
	if (err) {
	    printk("[AXP81X-MFD] try to clear irq failed!\n");
		return err;
	}

	/* mask and clear all IRQs */
	chip->irqs_enabled = 0xffffffff | (u64)0xffff << 32;
	chip->ops->disable_irqs(chip, chip->irqs_enabled);

	return 0;
}

static ssize_t axp81x_reg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u8 val;

	axp_read(dev,axp_reg_addr,&val);
	return sprintf(buf,"REG[%x]=%x\n",axp_reg_addr,val);
}

static ssize_t axp81x_reg_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	s32 tmp;
	u8 val;

	tmp = simple_strtoul(buf, NULL, 16);
	if( tmp < 256 )
		axp_reg_addr = tmp;
	else {
		val = tmp & 0x00FF;
		axp_reg_addr= (tmp >> 8) & 0x00FF;
		axp_write(dev,axp_reg_addr, val);
	}
	return count;
}

static u32 data2 = 2;
static ssize_t axp81x_regs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u8 val;
	s32 count = 0, i = 0;

	for (i=0; i<data2; i++) {
		axp_read(dev, axp_reg_addr+i, &val);
		count += sprintf(buf+count,"REG[0x%x]=0x%x\n",axp_reg_addr+i,val);
	}
	return count;
}

static ssize_t axp81x_regs_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	u32 data1 = 0;
	u8 val[3];
	char *endp;

	data1 = simple_strtoul(buf, &endp, 16);
	if (*endp != ' ') {
		printk("%s: %d\n", __func__, __LINE__);
		return -EINVAL;
	}
	buf = endp + 1;

	data2 = simple_strtoul(buf, &endp, 10);

	if( data1 < 256 )
		axp_reg_addr = data1;
	else {
		axp_reg_addr= (data1 >> 16) & 0xFF;
		val[0] = (data1 >> 8) & 0xFF;
		val[1] = axp_reg_addr + 1;
		val[2] = data1 & 0xFF;
		axp_writes(dev,axp_reg_addr,3,val);
	}
	return count;
}

static struct device_attribute axp81x_mfd_attrs[] = {
	AXP_MFD_ATTR(axp81x_reg),
	AXP_MFD_ATTR(axp81x_regs),
};

