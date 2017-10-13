#include "sunxi_rw_func.h"
/*analog register*/
u32 read_prcm_wvalue(u32 addr,void __iomem * ADDA_PR_CFG_REG)
{
	u32 reg;
	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1<<28);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1<<24);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1f<<16);
	reg |= (addr<<16);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= (0xff<<0);

	return reg;
}
EXPORT_SYMBOL_GPL(read_prcm_wvalue);

void write_prcm_wvalue(u32 addr, u32 val,void __iomem * ADDA_PR_CFG_REG)
{
	u32 reg;
	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1<<28);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1f<<16);
	reg |= (addr<<16);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0xff<<8);
	reg |= (val<<8);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1<<24);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1<<24);
	writel(reg, ADDA_PR_CFG_REG);
}
EXPORT_SYMBOL_GPL(write_prcm_wvalue);

/*digital*/
u32 codec_rdreg(void __iomem * address)
{
	return readl(address);
}
EXPORT_SYMBOL_GPL(codec_rdreg);

void codec_wrreg(void __iomem * address,u32 val)
{
	writel(val,address);
}
EXPORT_SYMBOL_GPL(codec_wrreg);

/**
* codec_wrreg_bits - update codec register bits
* @reg: codec register
* @mask: register mask
* @value: new value
*
* Writes new register value.
* Return 1 for change else 0.
*/
 u32 codec_wrreg_prcm_bits(void __iomem * ADDA_PR_CFG_REG,u32 reg, u32 mask, u32 value)
{
	u32 old, new;

	old	=	read_prcm_wvalue(reg,ADDA_PR_CFG_REG);
	new	=	(old & ~mask) | value;
	write_prcm_wvalue(reg,new,ADDA_PR_CFG_REG);

	return 0;
}
EXPORT_SYMBOL_GPL(codec_wrreg_prcm_bits);

/**
* codec_wrreg_bits - update codec register bits
* @reg: codec register
* @mask: register mask
* @value: new value
*
* Writes new register value.
* Return 1 for change else 0.
*/
u32 codec_wrreg_bits(void __iomem * address, u32 mask,u32 value)
{
	u32 old, new;

	old	=	codec_rdreg(address);
	new	=	(old & ~mask) | value;
	codec_wrreg(address,new);

	return 0;
}
EXPORT_SYMBOL_GPL(codec_wrreg_bits);

u32 codec_wr_control(void __iomem *  reg, u32 mask, u32 shift, u32 val)
{
	u32 reg_val;
	reg_val = val << shift;
	mask = mask << shift;
	codec_wrreg_bits(reg, mask, reg_val);
	return 0;
}
EXPORT_SYMBOL_GPL(codec_wr_control);

u32 audiodebug_reg_read(u32 reg)
{
	u32 reg_val = -1;
	void __iomem  *temp_address = ioremap(reg, 5);
	if (NULL ==temp_address) {
		pr_err("could not remap register memory\n");
		return -ENOMEM;
	} else
		reg_val = readl(temp_address);
	iounmap(temp_address);
	return reg_val;
}
EXPORT_SYMBOL_GPL(audiodebug_reg_read);