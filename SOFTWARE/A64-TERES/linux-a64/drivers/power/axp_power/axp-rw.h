#ifndef _LINUX_AXP_RW_H_
#define _LINUX_AXP_RW_H_

#include <linux/mfd/axp-mfd.h>
#ifdef CONFIG_SUNXI_ARISC
#include <linux/arisc/arisc.h>
#endif

static inline s32 __axp_read(unsigned char *devaddr, struct i2c_client *client,
				u32 reg, u8 *val, bool syncflag)
{
#ifdef	CONFIG_AXP_TWI_USED
	s32 ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading at 0x%02x\n", reg);
		return ret;
	}

	*val = (u8)ret;
	return 0;
#elif defined(CONFIG_SUNXI_ARISC)
	s32 ret;
	u8 addr = (u8)reg;
	u8 data = 0;
#if defined CONFIG_ARCH_SUN8IW1P1
	arisc_p2wi_block_cfg_t p2wi_data;
#else
	arisc_rsb_block_cfg_t rsb_data;
	u32 data_temp;
#endif

#if defined CONFIG_ARCH_SUN8IW1P1
	p2wi_data.len = 1;
	if(syncflag)
		p2wi_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
	else
		p2wi_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	p2wi_data.addr = &addr;
	p2wi_data.data = &data;

	ret = arisc_p2wi_read_block_data(&p2wi_data);
	if (ret != 0) {
		printk("failed read to 0x%02x\n", reg);
		return ret;
	}
#else
	rsb_data.len = 1;
	rsb_data.datatype = RSB_DATA_TYPE_BYTE;
	if(syncflag)
		rsb_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
	else
		rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	rsb_data.devaddr = *devaddr;
	rsb_data.regaddr = &addr;
	rsb_data.data = &data_temp;

	/* write axp registers */
	ret = arisc_rsb_read_block_data(&rsb_data);
	if (ret != 0) {
		printk("failed read to 0x%02x\n", reg);
		return ret;
	}
	data = (u8)data_temp;
#endif

	*val = data;
	return 0;
#else
	return 0;
#endif
}

static inline s32 __axp_reads(unsigned char *devaddr, struct i2c_client *client, int reg,
				 int len, u8 *val, bool syncflag)
{
#ifdef	CONFIG_AXP_TWI_USED
	s32 ret;

	ret = i2c_smbus_read_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading from 0x%02x\n", reg);
		return ret;
	}
	return 0;
#elif defined(CONFIG_SUNXI_ARISC)
	s32     ret, i, rd_len;
	u8 addr[AXP_TRANS_BYTE_MAX];
	u8 data[AXP_TRANS_BYTE_MAX];
	u8 *cur_data = val;
#if defined CONFIG_ARCH_SUN8IW1P1
	arisc_p2wi_block_cfg_t p2wi_data;
#else
	arisc_rsb_block_cfg_t rsb_data;
	u32 data_temp[AXP_TRANS_BYTE_MAX];
#endif

	/* fetch first register address */
	while (len > 0) {
		rd_len = min(len, AXP_TRANS_BYTE_MAX);
		for (i = 0; i < rd_len; i++) {
			addr[i] = reg++;
		}
#if defined CONFIG_ARCH_SUN8IW1P1
		p2wi_data.len = rd_len;
		if(syncflag)
			p2wi_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
		else
			p2wi_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
		p2wi_data.addr = addr;
		p2wi_data.data = data;

		ret = arisc_p2wi_read_block_data(&p2wi_data);
		if (ret != 0) {
			printk("failed reads to 0x%02x\n", reg);
			return ret;
		}
#else
		rsb_data.len = rd_len;
		rsb_data.datatype = RSB_DATA_TYPE_BYTE;
		if(syncflag)
			rsb_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
		else
			rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
		rsb_data.devaddr = *devaddr;
		rsb_data.regaddr = addr;
		rsb_data.data = data_temp;

		/* read axp registers */
		ret = arisc_rsb_read_block_data(&rsb_data);
		if (ret != 0) {
			printk("failed reads to 0x%02x\n", reg);
			return ret;
		}

		for(i = 0; i < rd_len; i++) {
			data[i] = (u8)data_temp[i];
		}
#endif
		/* copy data to user buffer */
		memcpy(cur_data, data, rd_len);
		cur_data = cur_data + rd_len;

		/* process next time read */
		len -= rd_len;
	}
	return 0;
#else
	return 0;
#endif
}

static inline s32 __axp_write(unsigned char *devaddr, struct i2c_client *client,
				 int reg, u8 val, bool syncflag)
{
#ifdef	CONFIG_AXP_TWI_USED
	s32 ret;

	axp_reg_debug(reg, 1, &val);
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing 0x%02x to 0x%02x\n",
				val, reg);
		return ret;
	}
	return 0;
#elif defined(CONFIG_SUNXI_ARISC)
	s32 ret;
	u8 addr = (u8)reg;

#if defined CONFIG_ARCH_SUN8IW1P1
	arisc_p2wi_block_cfg_t p2wi_data;
#else
	arisc_rsb_block_cfg_t rsb_data;
	u32 data;
#endif

	axp_reg_debug(reg, 1, &val);

#if defined CONFIG_ARCH_SUN8IW1P1
	p2wi_data.len = 1;
	if(syncflag)
		p2wi_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
	else
		p2wi_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	p2wi_data.addr = &addr;
	p2wi_data.data = &val;

	ret = arisc_p2wi_write_block_data(&p2wi_data);
	if (ret != 0) {
		printk("failed writing to 0x%02x\n", reg);
		return ret;
	}
#else
	data = (unsigned int)val;
	rsb_data.len = 1;
	rsb_data.datatype = RSB_DATA_TYPE_BYTE;
	if(syncflag)
		rsb_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
	else
		rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	rsb_data.devaddr = *devaddr;
	rsb_data.regaddr = &addr;
	rsb_data.data = &data;

	/* write axp registers */
	ret = arisc_rsb_write_block_data(&rsb_data);
	if (ret != 0) {
		printk("failed writing to 0x%02x\n", reg);
		return ret;
	}
#endif
	return 0;
#else
	return 0;
#endif
}


static inline s32 __axp_writes(unsigned char *devaddr, struct i2c_client *client, int reg,
				  int len, u8 *val, bool syncflag)
{
#ifdef	CONFIG_AXP_TWI_USED
	s32 ret;
	axp_reg_debug(reg, len, val);
	ret = i2c_smbus_write_i2c_block_data(client, reg, len, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writings to 0x%02x\n", reg);
		return ret;
	}
	return 0;
#elif defined(CONFIG_SUNXI_ARISC)
	s32     ret, i, first_flag, wr_len;
	u8 addr[AXP_TRANS_BYTE_MAX];
	u8 data[AXP_TRANS_BYTE_MAX];
#if defined CONFIG_ARCH_SUN8IW1P1
	arisc_p2wi_block_cfg_t p2wi_data;
#else
	arisc_rsb_block_cfg_t rsb_data;
	u32 data_temp[AXP_TRANS_BYTE_MAX];
#endif

	axp_reg_debug(reg, len, val);

	/* fetch first register address */
	first_flag = 1;
	addr[0] = (u8)reg;
	len = len + 1;	//+ first reg addr
	len = len >> 1;	//len = len / 2
	while (len > 0) {
		wr_len = min(len, AXP_TRANS_BYTE_MAX);
		for (i = 0; i < wr_len; i++) {
			if (first_flag) {
				/* skip the first reg addr */
				data[i] = *val++;
				first_flag = 0;
			} else {
				addr[i] = *val++;
				data[i] = *val++;
			}
		}
#if defined CONFIG_ARCH_SUN8IW1P1
		p2wi_data.len = wr_len;
		if(syncflag)
			p2wi_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
		else
			p2wi_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
		p2wi_data.addr = addr;
		p2wi_data.data = data;

		ret = arisc_p2wi_write_block_data(&p2wi_data);
		if (ret != 0) {
			printk("failed writings to 0x%02x\n", reg);
			return ret;
		}
#else
		for(i = 0; i < wr_len; i++) {
			data_temp[i] = (unsigned int)data[i];
		}
		rsb_data.len = wr_len;
		rsb_data.datatype = RSB_DATA_TYPE_BYTE;
		if(syncflag)
			rsb_data.msgattr = ARISC_MESSAGE_ATTR_HARDSYN;
		else
			rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
		rsb_data.devaddr = *devaddr;
		rsb_data.regaddr = addr;
		rsb_data.data = data_temp;

		/* write axp registers */
		ret = arisc_rsb_write_block_data(&rsb_data);
		if (ret != 0) {
			printk("failed writings to 0x%02x\n", reg);
			return ret;
		}
#endif
		/* process next time write */
		len -= wr_len;
	}
	return 0;
#else
	return 0;
#endif
}

#endif
