/*
 *  drivers/arisc/interfaces/arisc_rsb.c
 *
 * Copyright (c) 2013 Allwinner.
 * 2013-07-01 Written by superm (superm@allwinnertech.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../arisc_i.h"

#if (defined CONFIG_ARCH_SUN8IW3P1) || (defined CONFIG_ARCH_SUN8IW5P1) || (defined CONFIG_ARCH_SUN8IW6P1) || \
    (defined CONFIG_ARCH_SUN8IW7P1) || (defined CONFIG_ARCH_SUN8IW9P1) || (defined CONFIG_ARCH_SUN9IW1P1) || \
    (defined CONFIG_ARCH_SUN50IW1P1)

/*
 * used for indicate aduio codec been initialized,
 * modules like audio & trc mabye initialize,
 * but audio codec only can be initialize once
 */
static int audio_codec_init = 0;

/**
 * rsb read block data.
 * @cfg:    point of arisc_rsb_block_cfg struct;
 *
 * return: result, 0 - read register successed,
 *                !0 - read register failed or the len more then max len;
 */
int arisc_rsb_read_block_data(uint32_t *paras)
{
	int                   result;
	struct arisc_message *pmessage;

	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);

	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_RSB_READ_BLOCK_DATA;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	memcpy((void *)pmessage->paras, (const void *)paras, sizeof(pmessage->paras));

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	memcpy((void *)paras, (const void *)pmessage->paras, sizeof(pmessage->paras));

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}


/**
 * rsb write block data.
 * @cfg:    point of arisc_rsb_block_cfg struct;
 *
 * return: result, 0 - write register successed,
 *                !0 - write register failedor the len more then max len;
 */
int arisc_rsb_write_block_data(uint32_t *paras)
{
	int                   result;
	struct arisc_message *pmessage;

	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);

	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	/* initialize message */
	pmessage->type       = ARISC_RSB_WRITE_BLOCK_DATA;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	memcpy((void *)pmessage->paras, (const void *)paras, sizeof(pmessage->paras));

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}


/**
 * rsb read pmu reg.
 * @addr:  pmu reg addr;
 *
 * return: if read pmu reg successed, return data of pmu reg;
 *         if read pmu reg failed, return -1.
 */
uint8_t arisc_rsb_read_pmu_reg(uint32_t addr)
{
	int result;
	uint32_t paras[22];
	uint32_t data = 0;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]       |para[1]|para[2]   |para[3]|para[4]|para[5]|para[6]|
	 * |(len|datatype)|devaddr|regaddr0~3|data0  |data1  |data2  |data3  |
	 */
	memset((void *)paras, 0, sizeof(uint32_t) * 6);
	paras[0] = ((1 & 0xffff) | ((RSB_DATA_TYPE_BYTE << 16) & 0xffff0000));
	paras[1] = 0x2d;
	paras[2] = addr&0xff;

	result = arisc_rsb_read_block_data(paras);
	if (!result) {
		data = paras[3];
	} else {
		ARISC_ERR("arisc rsb read pmu reg 0x%x err\n", addr);
		return -1;
	}

	ARISC_INF("read pmu reg 0x%x:0x%x\n", addr, data);

	return data;
}


/**
 * rsb write pmu reg.
 * @addr: pmu reg addr;
 * @data: pmu reg data;
 *
 * return: result, 0 - write register successed,
 *                !0 - write register failedor the len more then max len;
 */
int arisc_rsb_write_pmu_reg(uint32_t addr, uint32_t data)
{
	int result;
	uint32_t paras[22];

	/*
	* package address and data to message->paras,
	* message->paras data layout:
	* |para[0]       |para[1]|para[2]   |para[3]|para[4]|para[5]|para[6]|
	* |(len|datatype)|devaddr|regaddr0~3|data0  |data1  |data2  |data3  |
	*/
	memset((void *)paras, 0, sizeof(uint32_t) * 6);
	paras[0] = ((1 & 0xffff) | ((RSB_DATA_TYPE_BYTE << 16) & 0xffff0000));
	paras[1] = 0x2d;
	paras[2] = addr&0xff;
	paras[3] = data&0xff;

	result = arisc_rsb_write_block_data(paras);
	if (result) {
		ARISC_ERR("arisc rsb write pmu reg 0x%x:0x%x err\n", addr, data);
	}
	ARISC_INF("write pmu reg 0x%x:0x%x\n", addr, data);

	return result;
}


/**
 * rsb bits operation sync.
 * @cfg:    point of arisc_rsb_bits_cfg struct;
 *
 * return: result, 0 - bits operation successed,
 *                !0 - bits operation failed, or the len more then max len;
 *
 * rsb clear bits internal:
 * data = rsb_read(regaddr);
 * data = data & (~mask);
 * rsb_write(regaddr, data);
 *
 * rsb set bits internal:
 * data = rsb_read(addr);
 * data = data | mask;
 * rsb_write(addr, data);
 *
 */
int rsb_bits_ops_sync(uint32_t *paras)
{
	int                   result;
	struct arisc_message *pmessage;

	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);

	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}
	/* initialize message */
	pmessage->type       = ARISC_RSB_BITS_OPS_SYNC;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	memcpy((void *)pmessage->paras, (const void *)paras, sizeof(pmessage->paras));

	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

/**
 * rsb set interface mode.
 * @devaddr:  rsb slave device address;
 * @regaddr:  register address of rsb slave device;
 * @data:     data which to init rsb slave device interface mode;
 *
 * return: result, 0 - set interface mode successed,
 *                !0 - set interface mode failed;
 */
int arisc_rsb_set_interface_mode(uint32_t devaddr, uint32_t regaddr, uint32_t data)
{
	int                   result;
	struct arisc_message *pmessage;

	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);

	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_RSB_SET_INTERFACE_MODE;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]|para[1]|para[2]|
	 * |devaddr|regaddr|data   |
	 */
	pmessage->paras[0] = devaddr;
	pmessage->paras[1] = regaddr;
	pmessage->paras[2] = data;
	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}

/**
 * rsb set runtime slave address.
 * @devaddr:  rsb slave device address;
 * @rtsaddr:  rsb slave device's runtime slave address;
 *
 * return: result, 0 - set rsb runtime address successed,
 *                !0 - set rsb runtime address failed;
 */
int arisc_rsb_set_rtsaddr(uint32_t devaddr, uint32_t rtsaddr)
{
	int                   result;
	struct arisc_message *pmessage;

	/* check audio codec has been initialized */
	if (devaddr == RSB_DEVICE_SADDR7) {
		if (audio_codec_init)
			return 0;
		else
			audio_codec_init = 1;
	}

	pmessage = arisc_message_allocate(ARISC_MESSAGE_ATTR_HARDSYN);

	if (pmessage == NULL) {
		ARISC_WRN("allocate message failed\n");
		return -ENOMEM;
	}

	/* initialize message */
	pmessage->type       = ARISC_RSB_SET_RTSADDR;
	pmessage->state      = ARISC_MESSAGE_INITIALIZED;
	pmessage->cb.handler = NULL;
	pmessage->cb.arg     = NULL;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]|para[1]|
	 * |devaddr|rtsaddr|
	 */
	pmessage->paras[0] = devaddr;
	pmessage->paras[1] = rtsaddr;
	/* send message use hwmsgbox */
	arisc_hwmsgbox_send_message(pmessage, ARISC_SEND_MSG_TIMEOUT);

	/* free message */
	result = pmessage->result;
	arisc_message_free(pmessage);

	return result;
}
#endif
