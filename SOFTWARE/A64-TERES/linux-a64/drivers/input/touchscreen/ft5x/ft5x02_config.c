#include "ft5x02_config.h"
//#include <linux/i2c/ft5x02_ts.h>

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#include "ini.h"
#include "ft5x02_ini_config.h"

//#define FTS_DBG
#ifdef FTS_DBG
#define DBG(fmt, args...) printk("[FTS]" fmt, ## args)
#else
#define DBG(fmt, args...) do{}while(0)
#endif

/*
*ft5x02_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x02_i2c_Read(struct i2c_client *client,  char * writebuf, int writelen, 
							char *readbuf, int readlen)
{
	int ret;

	if(writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				.addr	= client->addr,
				.flags	= 0,
				.len	= writelen,
				.buf	= writebuf,
			},
			{
				.addr	= client->addr,
				.flags	= I2C_M_RD,
				.len	= readlen,
				.buf	= readbuf,
			},
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			pr_err("function:%s. i2c read error: %d\n", __func__, ret);
	}
	else{
		struct i2c_msg msgs[] = {
			{
				.addr	= client->addr,
				.flags	= I2C_M_RD,
				.len	= readlen,
				.buf	= readbuf,
			},
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			pr_err("function:%s. i2c read error: %d\n", __func__, ret);
	}
	return ret;
}
/*
*write data by i2c 
*/
int ft5x02_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= writelen,
			.buf	= writebuf,
		},
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}

int ft5x02_write_reg(struct i2c_client * client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ft5x02_i2c_Write(client, buf, sizeof(buf));
}

int ft5x02_read_reg(struct i2c_client * client, u8 regaddr, u8 * regvalue)
{
	return ft5x02_i2c_Read(client, &regaddr, 1, regvalue, 1);
}

/*set tx order
*@txNO:		offset from tx order start
*@txNO1:	tx NO.
*/
static int ft5x02_set_tx_order(struct i2c_client * client, u8 txNO, u8 txNO1)
{
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1)
		ReCode = ft5x02_write_reg(client, FT5x02_REG_TX_ORDER_START + txNO,
						txNO1);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0)
			ReCode = ft5x02_write_reg(client,
					FT5x02_REG_TX_ORDER_START + txNO - FT5x02_TX_TEST_MODE_1,
					txNO1);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	return ReCode;
}

/*set tx order
*@txNO:		offset from tx order start
*@pTxNo:	return value of tx NO.
*/
static int ft5x02_get_tx_order(struct i2c_client * client, u8 txNO, u8 *pTxNo)
{
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1)
		ReCode = ft5x02_read_reg(client, FT5x02_REG_TX_ORDER_START + txNO,
						pTxNo);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if(ReCode >= 0)
			ReCode =  ft5x02_read_reg(client,
					FT5x02_REG_TX_ORDER_START + txNO - FT5x02_TX_TEST_MODE_1,
					pTxNo);	
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	return ReCode;
}

/*set tx cap
*@txNO: 	tx NO.
*@cap_value:	value of cap
*/
static int ft5x02_set_tx_cap(struct i2c_client * client, u8 txNO, u8 cap_value)
{
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1)
		ReCode = ft5x02_write_reg(client, FT5x02_REG_TX_CAP_START + txNO,
						cap_value);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0)
			ReCode = ft5x02_write_reg(client,
					FT5x02_REG_TX_CAP_START + txNO - FT5x02_TX_TEST_MODE_1,
					cap_value);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	return ReCode;
}

/*get tx cap*/
static int ft5x02_get_tx_cap(struct i2c_client * client, u8 txNO, u8 *pCap)
{
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1)
		ReCode =  ft5x02_read_reg(client, FT5x02_REG_TX_CAP_START + txNO,
					pCap);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0)
			ReCode = ft5x02_read_reg(client,
					FT5x02_REG_TX_CAP_START + txNO - FT5x02_TX_TEST_MODE_1,
					pCap);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	return ReCode;
}

/*set tx offset*/
static int ft5x02_set_tx_offset(struct i2c_client * client, u8 txNO, u8 offset_value)
{
	unsigned char temp=0;
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1) {
		ReCode = ft5x02_read_reg(client,
				FT5x02_REG_TX_OFFSET_START + (txNO>>1), &temp);
		if (ReCode >= 0) {
			if (txNO%2 == 0)
				ReCode = ft5x02_write_reg(client,
							FT5x02_REG_TX_OFFSET_START + (txNO>>1),
							(temp&0xf0) + (offset_value&0x0f));	
			else
				ReCode = ft5x02_write_reg(client,
							FT5x02_REG_TX_OFFSET_START + (txNO>>1),
							(temp&0x0f) + (offset_value<<4));	
		}
	} else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0) {
			ReCode = ft5x02_read_reg(client,
				FT5x02_REG_DEVICE_MODE+((txNO-FT5x02_TX_TEST_MODE_1)>>1),
				&temp);	/*enter Test mode 2*/
			if (ReCode >= 0) {
				if(txNO%2 == 0)
					ReCode = ft5x02_write_reg(client,
						FT5x02_REG_TX_OFFSET_START+((txNO-FT5x02_TX_TEST_MODE_1)>>1),
						(temp&0xf0)+(offset_value&0x0f));	
				else
					ReCode = ft5x02_write_reg(client,
						FT5x02_REG_TX_OFFSET_START+((txNO-FT5x02_TX_TEST_MODE_1)>>1),
						(temp&0xf0)+(offset_value<<4));	
			}
		}
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	
	return ReCode;
}

/*get tx offset*/
static int ft5x02_get_tx_offset(struct i2c_client * client, u8 txNO, u8 *pOffset)
{
	unsigned char temp=0;
	unsigned char ReCode = 0;
	if (txNO < FT5x02_TX_TEST_MODE_1)
		ReCode = ft5x02_read_reg(client,
				FT5x02_REG_TX_OFFSET_START + (txNO>>1), &temp);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0)
			ReCode = ft5x02_read_reg(client,
						FT5x02_REG_TX_OFFSET_START+((txNO-FT5x02_TX_TEST_MODE_1)>>1),
						&temp);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}

	if (ReCode >= 0)
		(txNO%2 == 0) ? (*pOffset = (temp&0x0f)) : (*pOffset = (temp>>4));
	return ReCode;
}

/*set rx order*/
static int ft5x02_set_rx_order(struct i2c_client * client, u8 rxNO, u8 rxNO1)
{
	unsigned char ReCode = 0;
	ReCode = ft5x02_write_reg(client, FT5x02_REG_RX_ORDER_START + rxNO,
						rxNO1);
	return ReCode;
}

/*get rx order*/
static int ft5x02_get_rx_order(struct i2c_client * client, u8 rxNO, u8 *prxNO1)
{
	unsigned char ReCode = 0;
	ReCode = ft5x02_read_reg(client, FT5x02_REG_RX_ORDER_START + rxNO,
						prxNO1);
	return ReCode;
}

/*set rx cap*/
static int ft5x02_set_rx_cap(struct i2c_client * client, u8 rxNO, u8 cap_value)
{
	unsigned char ReCode = 0;
	if (rxNO < FT5x02_RX_TEST_MODE_1)
		ReCode = ft5x02_write_reg(client, FT5x02_REG_RX_CAP_START + rxNO,
						cap_value);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if(ReCode >= 0)
			ReCode = ft5x02_write_reg(client,
					FT5x02_REG_RX_CAP_START + rxNO - FT5x02_RX_TEST_MODE_1,
					cap_value);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	
	return ReCode;
}

/*get rx cap*/
static int ft5x02_get_rx_cap(struct i2c_client * client, u8 rxNO, u8 *pCap)
{
	unsigned char ReCode = 0;
	if (rxNO < FT5x02_RX_TEST_MODE_1)
		ReCode = ft5x02_read_reg(client, FT5x02_REG_RX_CAP_START + rxNO,
						pCap);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if(ReCode >= 0)
			ReCode = ft5x02_read_reg(client,
					FT5x02_REG_RX_CAP_START + rxNO - FT5x02_RX_TEST_MODE_1,
					pCap);
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	
	return ReCode;
}

/*set rx offset*/
static int ft5x02_set_rx_offset(struct i2c_client * client, u8 rxNO, u8 offset_value)
{
	unsigned char temp=0;
	unsigned char ReCode = 0;
	if (rxNO < FT5x02_RX_TEST_MODE_1) {
		ReCode = ft5x02_read_reg(client,
				FT5x02_REG_RX_OFFSET_START + (rxNO>>1), &temp);
		if (ReCode >= 0) {
			if (rxNO%2 == 0)
				ReCode = ft5x02_write_reg(client,
							FT5x02_REG_RX_OFFSET_START + (rxNO>>1),
							(temp&0xf0) + (offset_value&0x0f));	
			else
				ReCode = ft5x02_write_reg(client,
							FT5x02_REG_RX_OFFSET_START + (rxNO>>1),
							(temp&0x0f) + (offset_value<<4));	
		}
	}
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0) {
			ReCode = ft5x02_read_reg(client,
				FT5x02_REG_DEVICE_MODE+((rxNO-FT5x02_RX_TEST_MODE_1)>>1),
				&temp);	/*enter Test mode 2*/
			if (ReCode >= 0) {
				if (rxNO%2 == 0)
					ReCode = ft5x02_write_reg(client,
						FT5x02_REG_RX_OFFSET_START+((rxNO-FT5x02_RX_TEST_MODE_1)>>1),
						(temp&0xf0)+(offset_value&0x0f));	
				else
					ReCode = ft5x02_write_reg(client,
						FT5x02_REG_RX_OFFSET_START+((rxNO-FT5x02_RX_TEST_MODE_1)>>1),
						(temp&0xf0)+(offset_value<<4));	
			}
		}
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}
	
	return ReCode;
}

/*get rx offset*/
static int ft5x02_get_rx_offset(struct i2c_client * client, u8 rxNO, u8 *pOffset)
{
	unsigned char temp = 0;
	unsigned char ReCode = 0;
	if (rxNO < FT5x02_RX_TEST_MODE_1)
		ReCode = ft5x02_read_reg(client,
				FT5x02_REG_RX_OFFSET_START + (rxNO>>1), &temp);
	else {
		ReCode = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
						FT5x02_REG_TEST_MODE_2<<4);	/*enter Test mode 2*/
		if (ReCode >= 0)
			ReCode = ft5x02_read_reg(client,
						FT5x02_REG_RX_OFFSET_START+((rxNO-FT5x02_RX_TEST_MODE_1)>>1),
						&temp);
		
		ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE,
			FT5x02_REG_TEST_MODE<<4);	/*enter Test mode*/
	}

	if (ReCode >= 0) {
		if (0 == (rxNO%2))
			*pOffset = (temp&0x0f);
		else
			*pOffset = (temp>>4);
	}
	
	return ReCode;
}

/*set tx num*/
static int ft5x02_set_tx_num(struct i2c_client *client, u8 txnum)
{
	return ft5x02_write_reg(client, FT5x02_REG_TX_NUM, txnum);
}

/*get tx num*/
static int ft5x02_get_tx_num(struct i2c_client *client, u8 *ptxnum)
{
	return ft5x02_read_reg(client, FT5x02_REG_TX_NUM, ptxnum);
}

/*set rx num*/
static int ft5x02_set_rx_num(struct i2c_client *client, u8 rxnum)
{
	return ft5x02_write_reg(client, FT5x02_REG_RX_NUM, rxnum);
}

/*get rx num*/
static int ft5x02_get_rx_num(struct i2c_client *client, u8 *prxnum)
{
	return ft5x02_read_reg(client, FT5x02_REG_RX_NUM, prxnum);
}

/*set resolution*/
static int ft5x02_set_Resolution(struct i2c_client *client, u16 x, u16 y)
{
	unsigned char cRet = 0;
	cRet &= ft5x02_write_reg(client,
			FT5x02_REG_RESOLUTION_X_H, ((unsigned char)(x>>8)));
	cRet &= ft5x02_write_reg(client,
			FT5x02_REG_RESOLUTION_X_L, ((unsigned char)(x&0x00ff)));

	cRet &= ft5x02_write_reg(client,
			FT5x02_REG_RESOLUTION_Y_H, ((unsigned char)(y>>8)));
	cRet &= ft5x02_write_reg(client,
			FT5x02_REG_RESOLUTION_Y_L, ((unsigned char)(y&0x00ff)));

	return cRet;
}

/*get resolution*/
static int ft5x02_get_Resolution(struct i2c_client *client,
			u16 *px, u16 *py)
{
	unsigned char cRet = 0, temp1 = 0, temp2 = 0;
	cRet &= ft5x02_read_reg(client,
			FT5x02_REG_RESOLUTION_X_H, &temp1);
	cRet &= ft5x02_read_reg(client,
			FT5x02_REG_RESOLUTION_X_L, &temp2);
	(*px) = (((u16)temp1) << 8) | ((u16)temp2);

	cRet &= ft5x02_read_reg(client,
			FT5x02_REG_RESOLUTION_Y_H, &temp1);
	cRet &= ft5x02_read_reg(client,
			FT5x02_REG_RESOLUTION_Y_L, &temp2);
	(*py) = (((u16)temp1) << 8) | ((u16)temp2);

	return cRet;
}


/*set voltage*/
static int ft5x02_set_vol(struct i2c_client *client, u8 Vol)
{
	return  ft5x02_write_reg(client, FT5x02_REG_VOLTAGE, Vol);
}

/*get voltage*/
static int ft5x02_get_vol(struct i2c_client *client, u8 *pVol)
{
	return ft5x02_read_reg(client, FT5x02_REG_VOLTAGE, pVol);
}

/*set gain*/
static int ft5x02_set_gain(struct i2c_client *client, u8 Gain)
{
	return ft5x02_write_reg(client, FT5x02_REG_GAIN, Gain);
}

/*get gain*/
static int ft5x02_get_gain(struct i2c_client *client, u8 *pGain)
{
	return ft5x02_read_reg(client, FT5x02_REG_GAIN, pGain);
}

static int ft5x02_set_face_detect_pre_value(struct i2c_client *client, u8 prevalue)
{
	return ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_PRE_VALUE,
			prevalue);
}

static int ft5x02_get_face_detect_pre_value(struct i2c_client *client, u8 *pprevalue)
{
	return ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_PRE_VALUE,
			pprevalue);
}

static int ft5x02_set_face_detect_num(struct i2c_client *client, u8 num)
{
	return ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_NUM,
			num);
}

static int ft5x02_get_face_detect_num(struct i2c_client *client, u8 *pnum)
{
	return ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_NUM,
			pnum);
}

static int ft5x02_set_face_detect_last_time(struct i2c_client *client, u8 lasttime_h, 
			u8 lasttime_l)
{
	int err = 0;
	u8 temp1 = 0, temp2 = 0;

	temp1 = lasttime_h;
	temp2 = lasttime_l;
	err = ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_LAST_TIME_H,
			temp1);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write face detect last time high.\n",
			__func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_LAST_TIME_L,
			temp2);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write face detect last time low.\n",
			__func__);
		return err;
	}
	return err;
}
static int ft5x02_get_face_detect_last_time(struct i2c_client *client, u8 *plasttime)
{
	int err = 0;
	u8 temp1 = 0, temp2 = 0;
	err = ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_LAST_TIME_H,
			&temp1);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not read face detect last time high.\n",
			__func__);
		return err;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_LAST_TIME_L,
			&temp2);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not read face detect last time low.\n",
			__func__);
		return err;
	}
	*plasttime = ((u16)temp1<<8) + (u16)temp2;

	return err;
}


static int ft5x02_set_peak_value_min(struct i2c_client *client, u8 min)
{
	return ft5x02_write_reg(client, FT5X02_REG_BIGAREA_PEAK_VALUE_MIN,
			min);
}

static int ft5x02_get_peak_value_min(struct i2c_client *client, u8 *pmin)
{
	return ft5x02_read_reg(client, FT5X02_REG_BIGAREA_PEAK_VALUE_MIN,
			pmin);
}

static int ft5x02_set_diff_value_over_num(struct i2c_client *client, u8 num)
{
	return ft5x02_write_reg(client, FT5X02_REG_BIGAREA_DIFF_VALUE_OVER_NUM,
			num);
}
static int ft5x02_get_diff_value_over_num(struct i2c_client *client, u8 *pnum)
{
	return ft5x02_read_reg(client, FT5X02_REG_BIGAREA_DIFF_VALUE_OVER_NUM,
			pnum);
}

static int ft5x02_set_point_auto_clear_time(struct i2c_client *client, u16 value)
{
	int err = 0;
	u8 temp1 = 0, temp2 = 0;

	temp1 = value >> 8;
	temp2 = value;
	err = ft5x02_write_reg(client, FT5X02_REG_BIGAREA_POINT_AUTO_CLEAR_TIME_H,
			temp1);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write point auot clean time high.\n",
			__func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_BIGAREA_POINT_AUTO_CLEAR_TIME_L,
			temp2);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write point auot clean time low.\n",
			__func__);
		return err;
	}
	return err;
}
static int ft5x02_get_point_auto_clear_time(struct i2c_client *client, u16 *pvalue)
{
	int err = 0;
	u8 temp1 = 0, temp2 = 0;
	err = ft5x02_read_reg(client, FT5X02_REG_BIGAREA_POINT_AUTO_CLEAR_TIME_H,
			&temp1);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write point auot clean time high.\n",
			__func__);
		return err;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_BIGAREA_POINT_AUTO_CLEAR_TIME_L,
			&temp2);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not write point auot clean time low.\n",
			__func__);
		return err;
	}
	*pvalue = ((u16)temp1<<8) + (u16)temp2;
	return err;

}

static int ft5x02_set_kx(struct i2c_client *client, u16 value)
{
	int err = 0;
	err = ft5x02_write_reg(client, FT5X02_REG_KX_H,
			value >> 8);
	if (err < 0)
		dev_err(&client->dev, "%s:set kx high failed\n",
				__func__);
	err = ft5x02_write_reg(client, FT5X02_REG_KX_L,
			value);
	if (err < 0)
		dev_err(&client->dev, "%s:set kx low failed\n",
				__func__);

	return err;
}

static int ft5x02_get_kx(struct i2c_client *client, u16 *pvalue)
{
	int err = 0;
	u8 tmp1, tmp2;
	err = ft5x02_read_reg(client, FT5X02_REG_KX_H,
			&tmp1);
	if (err < 0)
		dev_err(&client->dev, "%s:get kx high failed\n",
				__func__);
	err = ft5x02_read_reg(client, FT5X02_REG_KX_L,
			&tmp2);
	if (err < 0)
		dev_err(&client->dev, "%s:get kx low failed\n",
				__func__);

	*pvalue = ((u16)tmp1<<8) + (u16)tmp2;
	return err;
}
static int ft5x02_set_ky(struct i2c_client *client, u16 value)
{
	int err = 0;
	err = ft5x02_write_reg(client, FT5X02_REG_KY_H,
			value >> 8);
	if (err < 0)
		dev_err(&client->dev, "%s:set ky high failed\n",
				__func__);
	err = ft5x02_write_reg(client, FT5X02_REG_KY_L,
			value);
	if (err < 0)
		dev_err(&client->dev, "%s:set ky low failed\n",
				__func__);

	return err;
}

static int ft5x02_get_ky(struct i2c_client *client, u16 *pvalue)
{
	int err = 0;
	u8 tmp1, tmp2;
	err = ft5x02_read_reg(client, FT5X02_REG_KY_H,
			&tmp1);
	if (err < 0)
		dev_err(&client->dev, "%s:get ky high failed\n",
				__func__);
	err = ft5x02_read_reg(client, FT5X02_REG_KY_L,
			&tmp2);
	if (err < 0)
		dev_err(&client->dev, "%s:get ky low failed\n",
				__func__);

	*pvalue = ((u16)tmp1<<8) + (u16)tmp2;
	return err;
}
static int ft5x02_set_lemda_x(struct i2c_client *client, u8 value)
{
	return ft5x02_write_reg(client, FT5X02_REG_LEMDA_X,
			value);
}

static int ft5x02_get_lemda_x(struct i2c_client *client, u8 *pvalue)
{
	return ft5x02_read_reg(client, FT5X02_REG_LEMDA_X,
			pvalue);
}
static int ft5x02_set_lemda_y(struct i2c_client *client, u8 value)
{
	return ft5x02_write_reg(client, FT5X02_REG_LEMDA_Y,
			value);
}

static int ft5x02_get_lemda_y(struct i2c_client *client, u8 *pvalue)
{
	return ft5x02_read_reg(client, FT5X02_REG_LEMDA_Y,
			pvalue);
}
static int ft5x02_set_pos_x(struct i2c_client *client, u8 value)
{
	return ft5x02_write_reg(client, FT5X02_REG_DIRECTION,
			value);
}

static int ft5x02_get_pos_x(struct i2c_client *client, u8 *pvalue)
{
	return ft5x02_read_reg(client, FT5X02_REG_DIRECTION,
			pvalue);
}

static int ft5x02_set_scan_select(struct i2c_client *client, u8 value)
{
	return ft5x02_write_reg(client, FT5X02_REG_SCAN_SELECT,
			value);
}

static int ft5x02_get_scan_select(struct i2c_client *client, u8 *pvalue)
{
	return ft5x02_read_reg(client, FT5X02_REG_SCAN_SELECT,
			pvalue);
}

static int ft5x02_set_other_param(struct i2c_client *client)
{
	int err = 0;
	err = ft5x02_write_reg(client, FT5X02_REG_THGROUP, (u8)(g_param_ft5x02.ft5x02_THGROUP>>2));
	if (err < 0) {
		dev_err(&client->dev, "%s:write THGROUP failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_THPEAK, g_param_ft5x02.ft5x02_THPEAK);
	if (err < 0) {
		dev_err(&client->dev, "%s:write THPEAK failed.\n",
				__func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_THFALSE_TOUCH_PEAK,
			g_param_ft5x02.ft5x02_THFALSE_TOUCH_PEAK);
	if (err < 0) {
		dev_err(&client->dev, "%s:write THFALSE_TOUCH_PEAK failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_THDIFF, g_param_ft5x02.ft5x02_THDIFF);
	if (err < 0) {
		dev_err(&client->dev, "%s:write THDIFF failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_PWMODE_CTRL, 
		g_param_ft5x02.ft5x02_PWMODE_CTRL);
	if (err < 0) {
		dev_err(&client->dev, "%s:write PERIOD_CTRL failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_TIME_ENTER_MONITOR,
			g_param_ft5x02.ft5x02_TIME_ENTER_MONITOR);
	if (err < 0) {
		dev_err(&client->dev, "%s:write TIME_ENTER_MONITOR failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_PERIOD_ACTIVE,
			g_param_ft5x02.ft5x02_PERIOD_ACTIVE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write PERIOD_ACTIVE failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_STATISTICS_TX_NUM,
			g_param_ft5x02.ft5x02_FACE_DETECT_STATISTICS_TX_NUM);
	if (err < 0) {
		dev_err(&client->dev, "%s:write FACE_DETECT_STATISTICS_TX_NUM failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_PERIOD_MONITOR,
			g_param_ft5x02.ft5x02_PERIOD_MONITOR);
	if (err < 0) {
		dev_err(&client->dev, "%s:write PERIOD_MONITOR failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_AUTO_CLB_MODE,
			g_param_ft5x02.ft5x02_AUTO_CLB_MODE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write AUTO_CLB_MODE failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_MODE, g_param_ft5x02.ft5x02_MODE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write MODE failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_PMODE, g_param_ft5x02.ft5x02_PMODE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write PMODE failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_FIRMWARE_ID, g_param_ft5x02.ft5x02_FIRMWARE_ID);
	if (err < 0) {
		dev_err(&client->dev, "%s:write FIRMWARE_ID failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_STATE, g_param_ft5x02.ft5x02_STATE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write STATE failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_MAX_TOUCH_VALUE_HIGH,
			g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:write MAX_TOUCH_VALUE_HIGH failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_MAX_TOUCH_VALUE_LOW,
			g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write MAX_TOUCH_VALUE_LOW failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_FACE_DETECT_MODE,
			g_param_ft5x02.ft5x02_FACE_DETECT_MODE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write FACE_DETECT_MODE failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_DRAW_LINE_TH,
			g_param_ft5x02.ft5x02_DRAW_LINE_TH);
	if (err < 0) {
		dev_err(&client->dev, "%s:write DRAW_LINE_TH failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_DIFFDATA_HADDLE_VALUE,
			g_param_ft5x02.ft5x02_DIFFDATA_HADDLE_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write DIFFDATA_HADDLE_VALUE failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_ABNORMAL_DIFF_VALUE,
			g_param_ft5x02.ft5x02_ABNORMAL_DIFF_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ABNORMAL_DIFF_VALUE failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_ABNORMAL_DIFF_NUM,
			g_param_ft5x02.ft5x02_ABNORMAL_DIFF_NUM);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ABNORMAL_DIFF_NUM) failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_ABNORMAL_DIFF_LAST_FRAME,
			g_param_ft5x02.ft5x02_ABNORMAL_DIFF_LAST_FRAME);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ABNORMAL_DIFF_LAST_FRAME failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_POINTS_SUPPORTED,
			g_param_ft5x02.ft5x02_POINTS_SUPPORTED);
	if (err < 0) {
		dev_err(&client->dev, "%s:write POINTS_SUPPORTED failed.\n", __func__);
		return err;
	}
	/*******************************************************************/

	err = ft5x02_write_reg(client, FT5X02_REG_STATIC_TH,
			g_param_ft5x02.ft5x02_STATIC_TH);
	if (err < 0) {
		dev_err(&client->dev, "%s:write STATIC_TH failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_MID_SPEED_TH,
			g_param_ft5x02.ft5x02_MID_SPEED_TH);
	if (err < 0) {
		dev_err(&client->dev, "%s:write MID_SPEED_TH failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_HIGH_SPEED_TH,
			g_param_ft5x02.ft5x02_HIGH_SPEED_TH);
	if (err < 0) {
		dev_err(&client->dev, "%s:write HIGH_SPEED_TH failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_FILTER_FRAME_NOISE,
			g_param_ft5x02.ft5x02_FILTER_FRAME_NOISE);
	if (err < 0) {
		dev_err(&client->dev, "%s:write FILTER_FRAME_NOISE failed.\n", __func__);
		return err;
	}
	
	err = ft5x02_write_reg(client, FT5X02_REG_KX_LR_H,
			g_param_ft5x02.ft5x02_KX_LR>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:write REG_KX_LR_H failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_KX_LR_L,
			g_param_ft5x02.ft5x02_KX_LR);
	if (err < 0) {
		dev_err(&client->dev, "%s:write REG_KX_LR_L failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_KY_UD_H,
			g_param_ft5x02.ft5x02_KY_UD>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:write CCOFFSET_X failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_KY_UD_L,
			g_param_ft5x02.ft5x02_KY_UD);
	if (err < 0) {
		dev_err(&client->dev, "%s:write CCOFFSET_Y failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_POWERNOISE_FILTER_TH,
			g_param_ft5x02.ft5x02_POWERNOISE_FILTER_TH);
	if (err < 0) {
		dev_err(&client->dev, "%s:write POWERNOISE_FILTER_TH failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_ESD_FILTER_FRAME,
			g_param_ft5x02.ft5x02_ESD_FILTER_FRAME);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ft5x02_ESD_FILTER_FRAME failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_MOVSTH_I,
			g_param_ft5x02.ft5x02_MOVSTH_I);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ft5x02_MOVSTH_I failed.\n", __func__);
		return err;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_MOVSTH_N,
			g_param_ft5x02.ft5x02_MOVSTH_N);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ft5x02_MOVSTH_N failed.\n", __func__);
		return err;
	}
	
	return err;
}


static int ft5x02_get_other_param(struct i2c_client *client)
{
	int err = 0;
	u8 value = 0x00;
	err = ft5x02_read_reg(client, FT5X02_REG_THGROUP, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read THGROUP failed.\n", __func__);
		return err;
	} else {
		DBG("THGROUP=%02x\n", value<<2);
		if((value<<2) != g_param_ft5x02.ft5x02_THGROUP)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_THPEAK, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read THPEAK failed.\n",
				__func__);
		return err;
	} else {
		DBG("THPEAK=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_THPEAK)
		return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_THFALSE_TOUCH_PEAK,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read THFALSE_TOUCH_PEAK failed.\n", __func__);
		return err;
	}else {
		DBG("THFALSE_TOUCH_PEAK=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_THFALSE_TOUCH_PEAK)
		return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_THDIFF, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read THDIFF failed.\n", __func__);
		return err;
	}else {
		DBG("THDIFF=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_THDIFF)
		return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_PWMODE_CTRL, 
		&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read PERIOD_CTRL failed.\n", __func__);
		return err;
	}else {
		DBG("PWMODE_CTRL=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_PWMODE_CTRL)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_TIME_ENTER_MONITOR,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read TIME_ENTER_MONITOR failed.\n", __func__);
		return err;
	}else {
		DBG("TIME_ENTER_MONITOR=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_TIME_ENTER_MONITOR)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_PERIOD_ACTIVE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read PERIOD_ACTIVE failed.\n", __func__);
		return err;
	}else {
		DBG("PERIOD_ACTIVE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_PERIOD_ACTIVE)
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_STATISTICS_TX_NUM,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read FACE_DETECT_STATISTICS_TX_NUM failed.\n", __func__);
		return err;
	}else {
		DBG("FACE_DETECT_STATISTICS_TX_NUM=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FACE_DETECT_STATISTICS_TX_NUM)
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_PERIOD_MONITOR,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read PERIOD_MONITOR failed.\n", __func__);
		return err;
	}else {
		DBG("PERIOD_MONITOR=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_PERIOD_MONITOR)
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_AUTO_CLB_MODE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read AUTO_CLB_MODE failed.\n", __func__);
		return err;
	}else {
		DBG("AUTO_CLB_MODE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_AUTO_CLB_MODE)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_MODE, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read MODE failed.\n", __func__);
		return err;
	}else {
		DBG("MODE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_MODE)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_PMODE, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read PMODE failed.\n", __func__);
		return err;
	}else {
		DBG("PMODE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_PMODE)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_FIRMWARE_ID, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read FIRMWARE_ID failed.\n", __func__);
		return err;
	}else {
		DBG("FIRMWARE_ID=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FIRMWARE_ID)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_STATE, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read STATE failed.\n", __func__);
		return err;
	}else {
		DBG("STATE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_STATE)
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_MAX_TOUCH_VALUE_HIGH,
			&value);//g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:read MAX_TOUCH_VALUE_HIGH failed.\n", __func__);
		return err;
	}else {
		DBG("MAX_TOUCH_VALUE_HIGH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE>>8)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_MAX_TOUCH_VALUE_LOW,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read MAX_TOUCH_VALUE_LOW failed.\n", __func__);
		return err;
	}else {
		DBG("MAX_TOUCH_VALUE_LOW=%02x\n", value);
		if(value != (g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE&0x00FF))
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_FACE_DETECT_MODE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read FACE_DETECT_MODE failed.\n", __func__);
		return err;
	}else {
		DBG("FACE_DETECT_MODE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FACE_DETECT_MODE)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_DRAW_LINE_TH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read DRAW_LINE_TH failed.\n", __func__);
		return err;
	}else {
		DBG("DRAW_LINE_TH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_DRAW_LINE_TH)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_DIFFDATA_HADDLE_VALUE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read DIFFDATA_HADDLE_VALUE failed.\n", __func__);
		return err;
	}else {
		DBG("DIFFDATA_HADDLE_VALUE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_DIFFDATA_HADDLE_VALUE)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_ABNORMAL_DIFF_VALUE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ABNORMAL_DIFF_VALUE failed.\n", __func__);
		return err;
	}else {
		DBG("ABNORMAL_DIFF_VALUE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_ABNORMAL_DIFF_VALUE)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_ABNORMAL_DIFF_NUM,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ABNORMAL_DIFF_NUM failed.\n", __func__);
		return err;
	}else {
		DBG("ABNORMAL_DIFF_NUM=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_ABNORMAL_DIFF_NUM)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_ABNORMAL_DIFF_LAST_FRAME,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ABNORMAL_DIFF_LAST_FRAME failed.\n", __func__);
		return err;
	}else {
		DBG("ABNORMAL_DIFF_LAST_FRAME=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_ABNORMAL_DIFF_LAST_FRAME)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_POINTS_SUPPORTED,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read POINTS_SUPPORTED failed.\n", __func__);
		return err;
	}else {
		DBG("POINTS_SUPPORTED=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_POINTS_SUPPORTED)
			return -1;
	}
	/*******************************************************************/

	err = ft5x02_read_reg(client, FT5X02_REG_STATIC_TH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read STATIC_TH failed.\n", __func__);
		return err;
	}else {
		DBG("STATIC_TH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_STATIC_TH)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_MID_SPEED_TH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read MID_SPEED_TH failed.\n", __func__);
		return err;
	}else {
		DBG("MID_SPEED_TH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_MID_SPEED_TH)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_HIGH_SPEED_TH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read HIGH_SPEED_TH failed.\n", __func__);
		return err;
	}else {
		DBG("HIGH_SPEED_TH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_HIGH_SPEED_TH)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_FILTER_FRAME_NOISE,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read FILTER_FRAME_NOISE failed.\n", __func__);
		return err;
	}else {
		DBG("FILTER_FRAME_NOISE=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FILTER_FRAME_NOISE)
			return -1;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_KX_LR_H,
			&value);//g_param_ft5x02.ft5x02_KX_LR>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:read REG_KX_LR_H failed.\n", __func__);
		return err;
	}else {
		DBG("KX_LR_H=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_KX_LR>>8)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_KX_LR_L,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read REG_KX_LR_L failed.\n", __func__);
		return err;
	}else {
		DBG("KX_LR_L=%02x\n", value);
		if(value != (g_param_ft5x02.ft5x02_KX_LR&0x00FF))
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_KY_UD_H,
			&value);//g_param_ft5x02.ft5x02_KY_UD>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ft5x02_KY_UD failed.\n", __func__);
		return err;
	}else {
		DBG("ft5x02_KY_UD=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_KY_UD>>8)
			return -1;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_KY_UD_L,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ft5x02_KY_UD failed.\n", __func__);
		return err;
	}else {
		DBG("ft5x02_KY_UD=%02x\n", value);
		if(value != (g_param_ft5x02.ft5x02_KY_UD&0x00FF))
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_POWERNOISE_FILTER_TH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read POWERNOISE_FILTER_TH failed.\n", __func__);
		return err;
	}else {
		DBG("POWERNOISE_FILTER_TH=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_POWERNOISE_FILTER_TH)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_ESD_FILTER_FRAME,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ESD_FILTER_FRAME failed.\n", __func__);
		return err;
	}else {
		DBG("ESD_FILTER_FRAME=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_ESD_FILTER_FRAME)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_MOVSTH_I,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read MOVSTH_I failed.\n", __func__);
		return err;
	}else {
		DBG("MOVSTH_I=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_MOVSTH_I)
			return -1;
	}

	err = ft5x02_read_reg(client, FT5X02_REG_MOVSTH_N,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ft5x02_MOVSTH_N failed.\n", __func__);
		return err;
	}else {
		DBG("MOVSTH_N=%02x\n", value);
		if(value != g_param_ft5x02.ft5x02_MOVSTH_N)
			return -1;
	}
	
	return err;
}

int ft5x02_get_ic_param(struct i2c_client *client)
{
	int err = 0;
	int i = 0;
	u8 value = 0x00;
	u16 xvalue = 0x0000, yvalue = 0x0000;
	
	/*enter factory mode*/
	err = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE, FT5x02_FACTORYMODE_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:enter factory mode failed.\n", __func__);
		goto ERR_EXIT;
	}
	
	for (i = 0; i < g_ft5x02_tx_num; i++) {
		DBG("tx%d:", i);
		/*get tx order*/
		err = ft5x02_get_tx_order(client, i, &value);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not get tx%d order.\n",
					__func__, i);
			goto ERR_EXIT;
		}
		DBG("order=%d ", value);
		if(value != g_ft5x02_tx_order[i])
			goto ERR_EXIT;
		/*get tx cap*/
		err = ft5x02_get_tx_cap(client, i, &value);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not get tx%d cap.\n",
					__func__, i);
			goto ERR_EXIT;
		}
		DBG("cap=%02x\n", value);
		if(value != g_ft5x02_tx_cap[i])
			goto ERR_EXIT;
	}
	/*get tx offset*/
	err = ft5x02_get_tx_offset(client, 0, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get tx 0 offset.\n",
				__func__);
		goto ERR_EXIT;
	} else {
		DBG("tx offset = %02x\n", value);
		if(value != g_ft5x02_tx_offset)
			goto ERR_EXIT;
	}

	/*get rx offset and cap*/
	for (i = 0; i < g_ft5x02_rx_num; i++) {
		/*get rx order*/
		DBG("rx%d:", i);
		err = ft5x02_get_rx_order(client, i, &value);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not get rx%d order.\n",
					__func__, i);
			goto ERR_EXIT;
		}
		DBG("order=%d ", value);
		if(value != g_ft5x02_rx_order[i])
			goto ERR_EXIT;
		/*get rx cap*/
		err = ft5x02_get_rx_cap(client, i, &value);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not get rx%d cap.\n",
					__func__, i);
			goto ERR_EXIT;
		}
		DBG("cap=%02x ", value);
		if(value != g_ft5x02_rx_cap[i])
			goto ERR_EXIT;
		
		err = ft5x02_get_rx_offset(client, i, &value);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not get rx offset.\n",
				__func__);
			goto ERR_EXIT;
		}
		DBG("offset=%02x\n", value);
		#if 0
		if(i%2 == 0) {
			if(value != ((g_ft5x02_rx_offset[i/2]&0xF0)>>4))
				goto ERR_EXIT;
		}else{
			if(value != ((g_ft5x02_rx_offset[i/2]&0x0F)))
				goto ERR_EXIT;
		}
		#endif
	}

	/*get scan select*/
	err = ft5x02_get_scan_select(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get scan select.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("scan select = %02x\n", value);
		if(value != g_ft5x02_scanselect)
			goto ERR_EXIT;
	}
	
	/*get tx number*/
	err = ft5x02_get_tx_num(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get tx num.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("tx num = %02x\n", value);
		if(value != g_ft5x02_tx_num)
			goto ERR_EXIT;
	}
	/*get rx number*/
	err = ft5x02_get_rx_num(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get rx num.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("rx num = %02x\n", value);
		if(value != g_ft5x02_rx_num)
			goto ERR_EXIT;
	}
	
	/*get gain*/
	err = ft5x02_get_gain(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get gain.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("gain = %02x\n", value);
		if(value != g_ft5x02_gain)
			goto ERR_EXIT;
	}
	/*get voltage*/
	err = ft5x02_get_vol(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get voltage.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("voltage = %02x\n", value);
		if(value != g_ft5x02_voltage)
			goto ERR_EXIT;
	}
	
	err = ft5x02_read_reg(client, FT5X02_REG_ADC_TARGET_HIGH,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ADC_TARGET_HIGH failed.\n", __func__);
		goto ERR_EXIT;
	} else {
		DBG("ADC_TARGET_HIGH = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_ADC_TARGET>>8)
			goto ERR_EXIT;
	}
	err = ft5x02_read_reg(client, FT5X02_REG_ADC_TARGET_LOW,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:read ADC_TARGET_LOW failed.\n", __func__);
		goto ERR_EXIT;
	}else {
		DBG("ADC_TARGET_LOW = %02x\n", value);
		if(value != (g_param_ft5x02.ft5x02_ADC_TARGET&0x00FF))
			goto ERR_EXIT;
	}
	
//RETURN_WORK:	
	/*enter work mode*/
	err = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE, FT5x02_WORKMODE_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:enter work mode failed.\n", __func__);
		goto ERR_EXIT;
	}

	/*get resolution*/
	err = ft5x02_get_Resolution(client, &xvalue, &yvalue);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get resolution.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("resolution X = %d Y = %d\n", xvalue, yvalue);
		if(xvalue != g_param_ft5x02.ft5x02_RESOLUTION_X ||
			yvalue != g_param_ft5x02.ft5x02_RESOLUTION_Y)
			goto ERR_EXIT;
	}
	/*get face detect pre value*/
	err = ft5x02_get_face_detect_pre_value(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev,
				"%s:could not get face detect pre value.\n",
				__func__);
		goto ERR_EXIT;
	} else {
		DBG("FACE_DETECT_PRE_VALUE = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FACE_DETECT_PRE_VALUE)
			goto ERR_EXIT;
	}
	/*get face detect num*/
	err = ft5x02_get_face_detect_num(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get face detect num.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("FACE_DETECT_NUM = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_FACE_DETECT_NUM)
			goto ERR_EXIT;
	}
	/*get face detect last time*/
	err = ft5x02_get_face_detect_last_time(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev,
			"%s:could not get face detect last time.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("FACE_DETECT_LAST_TIME = %d\n", value);
		if(value != g_param_ft5x02.ft5x02_FACE_DETECT_LAST_TIME)
			goto ERR_EXIT;
	}
	
	/*get min peak value*/
	err = ft5x02_get_peak_value_min(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get min peak value.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("BIGAREA_PEAK_VALUE_MIN = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_BIGAREA_PEAK_VALUE_MIN)
			goto ERR_EXIT;
	}
	/*get diff value over num*/
	err = ft5x02_get_diff_value_over_num(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get diff value over num.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("BIGAREA_DIFF_VALUE_OVER_NUM = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_BIGAREA_DIFF_VALUE_OVER_NUM)
			goto ERR_EXIT;
	}
	
	/*get point auto clear time*/
	err = ft5x02_get_point_auto_clear_time(client,
			&xvalue);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get point auto clear time.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("BIGAREA_POINT_AUTO_CLEAR_TIME = %d\n", xvalue);
		if(xvalue != g_param_ft5x02.ft5x02_BIGAREA_POINT_AUTO_CLEAR_TIME)
			goto ERR_EXIT;
	}
	/*get kx*/
	err = ft5x02_get_kx(client, &xvalue);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get kx.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("KX = %d\n", xvalue);
		if(xvalue != g_param_ft5x02.ft5x02_KX)
			goto ERR_EXIT;
	}
	/*get ky*/
	err = ft5x02_get_ky(client, &xvalue);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get ky.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("ky = %d\n", xvalue);
		if(xvalue != g_param_ft5x02.ft5x02_KY)
			goto ERR_EXIT;
	}
	/*get lemda x*/
	err = ft5x02_get_lemda_x(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get lemda x.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("lemda x = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_LEMDA_X)
			goto ERR_EXIT;
	}
	/*get lemda y*/
	err = ft5x02_get_lemda_y(client,
			&value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get lemda y.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("lemda y = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_LEMDA_Y)
			goto ERR_EXIT;
	}
	/*get pos x*/
	err = ft5x02_get_pos_x(client, &value);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not get pos x.\n",
			__func__);
		goto ERR_EXIT;
	} else {
		DBG("pos x = %02x\n", value);
		if(value != g_param_ft5x02.ft5x02_DIRECTION)
			goto ERR_EXIT;
	}

	err = ft5x02_get_other_param(client);
	
ERR_EXIT:
	return err;
}

int ft5x02_Init_IC_Param(struct i2c_client *client)
{
	int err = 0;
	int i = 0;
	
	/*enter factory mode*/
	err = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE, FT5x02_FACTORYMODE_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:enter factory mode failed.\n", __func__);
		goto RETURN_WORK;
	}
	
	for (i = 0; i < g_ft5x02_tx_num; i++) {
		if (g_ft5x02_tx_order[i] != 0xFF) {
			/*set tx order*/
			err = ft5x02_set_tx_order(client, i, g_ft5x02_tx_order[i]);
			if (err < 0) {
				dev_err(&client->dev, "%s:could not set tx%d order.\n",
						__func__, i);
				goto RETURN_WORK;
			}
		}
		/*set tx cap*/
		err = ft5x02_set_tx_cap(client, i, g_ft5x02_tx_cap[i]);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not set tx%d cap.\n",
					__func__, i);
			goto RETURN_WORK;
		}
	}
	/*set tx offset*/
	err = ft5x02_set_tx_offset(client, 0, g_ft5x02_tx_offset);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set tx 0 offset.\n",
				__func__);
		goto RETURN_WORK;
	}

	/*set rx offset and cap*/
	for (i = 0; i < g_ft5x02_rx_num; i++) {
		/*set rx order*/
		err = ft5x02_set_rx_order(client, i, g_ft5x02_rx_order[i]);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not set rx%d order.\n",
					__func__, i);
			goto RETURN_WORK;
		}
		/*set rx cap*/
		err = ft5x02_set_rx_cap(client, i, g_ft5x02_rx_cap[i]);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not set rx%d cap.\n",
					__func__, i);
			goto RETURN_WORK;
		}
	}
	for (i = 0; i < g_ft5x02_rx_num/2; i++) {
		err = ft5x02_set_rx_offset(client, i*2, g_ft5x02_rx_offset[i]>>4);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not set rx offset.\n",
				__func__);
			goto RETURN_WORK;
		}
		err = ft5x02_set_rx_offset(client, i*2+1, g_ft5x02_rx_offset[i]&0x0F);
		if (err < 0) {
			dev_err(&client->dev, "%s:could not set rx offset.\n",
				__func__);
			goto RETURN_WORK;
		}
	}

	/*set scan select*/
	err = ft5x02_set_scan_select(client, g_ft5x02_scanselect);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set scan select.\n",
			__func__);
		goto RETURN_WORK;
	}
	
	/*set tx number*/
	err = ft5x02_set_tx_num(client, g_ft5x02_tx_num);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set tx num.\n",
			__func__);
		goto RETURN_WORK;
	}
	/*set rx number*/
	err = ft5x02_set_rx_num(client, g_ft5x02_rx_num);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set rx num.\n",
			__func__);
		goto RETURN_WORK;
	}
	
	/*set gain*/
	err = ft5x02_set_gain(client, g_ft5x02_gain);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set gain.\n",
			__func__);
		goto RETURN_WORK;
	}
	/*set voltage*/
	err = ft5x02_set_vol(client, g_ft5x02_voltage);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set voltage.\n",
			__func__);
		goto RETURN_WORK;
	}

	err = ft5x02_write_reg(client, FT5X02_REG_ADC_TARGET_HIGH,
			g_param_ft5x02.ft5x02_ADC_TARGET>>8);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ADC_TARGET_HIGH failed.\n", __func__);
		return err;
	}
	err = ft5x02_write_reg(client, FT5X02_REG_ADC_TARGET_LOW,
			g_param_ft5x02.ft5x02_ADC_TARGET);
	if (err < 0) {
		dev_err(&client->dev, "%s:write ADC_TARGET_LOW failed.\n", __func__);
		return err;
	}

RETURN_WORK:	
	/*enter work mode*/
	err = ft5x02_write_reg(client, FT5x02_REG_DEVICE_MODE, FT5x02_WORKMODE_VALUE);
	if (err < 0) {
		dev_err(&client->dev, "%s:enter work mode failed.\n", __func__);
		goto ERR_EXIT;
	}

	/*set resolution*/
	err = ft5x02_set_Resolution(client, g_param_ft5x02.ft5x02_RESOLUTION_X,
				g_param_ft5x02.ft5x02_RESOLUTION_Y);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set resolution.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set face detect pre value*/
	err = ft5x02_set_face_detect_pre_value(client,
			g_param_ft5x02.ft5x02_FACE_DETECT_PRE_VALUE);
	if (err < 0) {
		dev_err(&client->dev,
				"%s:could not set face detect pre value.\n",
				__func__);
		goto ERR_EXIT;
	}
	/*set face detect num*/
	err = ft5x02_set_face_detect_num(client,
			g_param_ft5x02.ft5x02_FACE_DETECT_NUM);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set face detect num.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set face detect last time*/
	err = ft5x02_set_face_detect_last_time(client,
			g_param_ft5x02.ft5x02_FACE_DETECT_LAST_TIME>>8,
			g_param_ft5x02.ft5x02_FACE_DETECT_LAST_TIME);
	if (err < 0) {
		dev_err(&client->dev,
			"%s:could not set face detect last time.\n",
			__func__);
		goto ERR_EXIT;
	}

	/*set min peak value*/
	err = ft5x02_set_peak_value_min(client,
			g_param_ft5x02.ft5x02_BIGAREA_PEAK_VALUE_MIN);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set min peak value.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set diff value over num*/
	err = ft5x02_set_diff_value_over_num(client,
			g_param_ft5x02.ft5x02_BIGAREA_DIFF_VALUE_OVER_NUM);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set diff value over num.\n",
			__func__);
		goto ERR_EXIT;
	}

	/*set point auto clear time*/
	err = ft5x02_set_point_auto_clear_time(client,
			g_param_ft5x02.ft5x02_BIGAREA_POINT_AUTO_CLEAR_TIME);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set point auto clear time.\n",
			__func__);
		goto ERR_EXIT;
	}

	/*set kx*/
	err = ft5x02_set_kx(client, g_param_ft5x02.ft5x02_KX);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set kx.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set ky*/
	err = ft5x02_set_ky(client, g_param_ft5x02.ft5x02_KY);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set ky.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set lemda x*/
	err = ft5x02_set_lemda_x(client,
			g_param_ft5x02.ft5x02_LEMDA_X);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set lemda x.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set lemda y*/
	err = ft5x02_set_lemda_y(client,
			g_param_ft5x02.ft5x02_LEMDA_Y);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set lemda y.\n",
			__func__);
		goto ERR_EXIT;
	}
	/*set pos x*/
	err = ft5x02_set_pos_x(client, g_param_ft5x02.ft5x02_DIRECTION);
	if (err < 0) {
		dev_err(&client->dev, "%s:could not set pos x.\n",
			__func__);
		goto ERR_EXIT;
	}

	err = ft5x02_set_other_param(client);
	
ERR_EXIT:
	return err;
}


char dst[512];
static char * ft5x02_sub_str(char * src, int n)
{
	char *p = src;
	int i;
	int m = 0;
	int len = strlen(src);

	while (n >= 1 && m <= len) {
		i = 0;
		dst[10] = ' ';
		n--;
		while ( *p != ',' && *p != ' ') {
			dst[i++] = *(p++);
			m++;
			if (i >= len)
				break;
		}
		dst[i++] = '\0';
		p++;
	}
	return dst;
}
static int ft5x02_GetInISize(char *config_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[128];
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "%s%s", FT5X02_INI_FILEPATH, config_name);

	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);

	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}

static int ft5x0x_ReadInIData(char *config_name,
			      char *config_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s%s", FT5X02_INI_FILEPATH, config_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, config_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}

int ft5x02_Get_Param_From_Ini(char *config_name)
{
	char key[64];
	char value[512];
	char section[64];
	int i = 0;//,ret=0;
	int j = 0;
	char *filedata = NULL;
	unsigned char legal_byte1 = 0x00;
	unsigned char legal_byte2 = 0x00;

	int inisize = ft5x02_GetInISize(config_name);
	
	if (inisize <= 0) {
		pr_err("%s ERROR:Get firmware size failed\n",
					__func__);
		return -EIO;
	}

	filedata = kmalloc(inisize + 1, GFP_ATOMIC);
		
	if (ft5x0x_ReadInIData(config_name, filedata)) {
		pr_err("%s() - ERROR: request_firmware failed\n",
					__func__);
		kfree(filedata);
		return -EIO;
	}

	/*check ini  if  it is illegal*/
	sprintf(section, "%s", FT5X02_APP_LEGAL);
	sprintf(key, "%s", FT5X02_APP_LEGAL_BYTE_1_STR);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	legal_byte1 = atoi(value);
	DBG("legal_byte1=%s\n", value);
	sprintf(key, "%s", FT5X02_APP_LEGAL_BYTE_2_STR);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	legal_byte2 = atoi(value);
	DBG("lega2_byte1=%s\n", value);
	if(FT5X02_APP_LEGAL_BYTE_1_VALUE == legal_byte1 &&
		FT5X02_APP_LEGAL_BYTE_2_VALUE == legal_byte2)
		DBG("the ini file is valid\n");
	else {
		pr_err("[FTS]-----the ini file is invalid!please check it.\n");
		goto ERROR_RETURN;
	}

	/*get ini param*/
	sprintf(section, "%s", FT5X02_APP_NAME);
		
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_KX = atoi(value);
	DBG("ft5x02_KX=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_KY = atoi(value);
	DBG("ft5x02_KY=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_LEMDA_X = atoi(value);
	DBG("ft5x02_LEMDA_X=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_LEMDA_Y = atoi(value);
	DBG("ft5x02_LEMDA_Y=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_RESOLUTION_X = atoi(value);
	DBG("ft5x02_RESOLUTION_X=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_RESOLUTION_Y = atoi(value);
	DBG("ft5x02_RESOLUTION_Y=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_DIRECTION= atoi(value);
	DBG("ft5x02_DIRECTION=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FACE_DETECT_PRE_VALUE = atoi(value);
	DBG("ft5x02_FACE_DETECT_PRE_VALUE=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FACE_DETECT_NUM = atoi(value);
	DBG("ft5x02_FACE_DETECT_NUM=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_BIGAREA_PEAK_VALUE_MIN = atoi(value);/*The min value to be decided as the big point*/
	DBG("ft5x02_BIGAREA_PEAK_VALUE_MIN=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_BIGAREA_DIFF_VALUE_OVER_NUM = atoi(value);/*The min big points of the big area*/
	DBG("ft5x02_BIGAREA_DIFF_VALUE_OVER_NUM=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_BIGAREA_POINT_AUTO_CLEAR_TIME = atoi(value);/*3000ms*/
	DBG("ft5x02_BIGAREA_POINT_AUTO_CLEAR_TIME=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FACE_DETECT_LAST_TIME = atoi(value);
	DBG("ft5x02_FACE_DETECT_LAST_TIME=%s\n", value);
	
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_MODE = atoi(value);
	DBG("ft5x02_MODE=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_PMODE = atoi(value);
	DBG("ft5x02_PMODE=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FIRMWARE_ID = atoi(value);
	DBG("ft5x02_FIRMWARE_ID=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_STATE = atoi(value);
	DBG("ft5x02_STATE=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_CUSTOMER_ID = atoi(value);
	DBG("ft5x02_CUSTOM_ID=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_PERIOD_ACTIVE = atoi(value);
	DBG("ft5x02_PERIOD_ACTIVE=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FACE_DETECT_STATISTICS_TX_NUM = atoi(value);
	DBG("ft5x02_FACE_DETECT_STATISTICS_TX_NUM=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_THGROUP = atoi(value);
	DBG("ft5x02_THGROUP=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_THPEAK = atoi(value);
	DBG("ft5x02_THPEAK=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_FACE_DETECT_MODE = atoi(value);
	DBG("ft5x02_FACE_DETECT_MODE=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_MAX_TOUCH_VALUE = atoi(value);
	DBG("ft5x02_MAX_TOUCH_VALUE=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_THFALSE_TOUCH_PEAK = atoi(value);
	DBG("ft5x02_THFALSE_TOUCH_PEAK=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_THDIFF = atoi(value);
	DBG("ft5x02_THDIFF=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_PWMODE_CTRL= atoi(value);
	DBG("ft5x02_PWMODE_CTRL=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_TIME_ENTER_MONITOR = atoi(value);
	DBG("ft5x02_TIME_ENTER_MONITOR=%s\n", value);


	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;

	i++;
	g_param_ft5x02.ft5x02_PERIOD_MONITOR = atoi(value);
	DBG("ft5x02_PERIOD_MONITOR=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_AUTO_CLB_MODE = atoi(value);
	DBG("ft5x02_AUTO_CLB_MODE=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_DRAW_LINE_TH = atoi(value);
	DBG("ft5x02_DRAW_LINE_TH=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_DIFFDATA_HADDLE_VALUE = atoi(value);
	DBG("ft5x02_DIFFDATA_HADDLE_VALUE=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_ABNORMAL_DIFF_VALUE = atoi(value);
	DBG("ft5x02_ABNORMAL_DIFF_VALUE=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_ABNORMAL_DIFF_NUM = atoi(value);
	DBG("ft5x02_ABNORMAL_DIFF_NUM=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_ABNORMAL_DIFF_LAST_FRAME = atoi(value);
	DBG("ft5x02_ABNORMAL_DIFF_LAST_FRAME=%s\n", value);
	

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_POINTS_SUPPORTED= atoi(value);
	DBG("ft5x02_POINTS_SUPPORTED=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_MOVSTH_I= atoi(value);
	DBG("ft5x02_MOVSTH_I=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_MOVSTH_N= atoi(value);
	DBG("ft5x02_MOVSTH_N=%s\n", value);
	
/****************************************************************/
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_STATIC_TH = atoi(value);
	DBG("ft5x02_STATIC_TH=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_MID_SPEED_TH = atoi(value);
	DBG("ft5x02_MID_SPEED_TH=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_HIGH_SPEED_TH = atoi(value);
	DBG("ft5x02_HIGH_SPEED_TH=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_param_ft5x02.ft5x02_START_RX = atoi(value);
	DBG("ft5x02_START_RX=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_ADC_TARGET = atoi(value);
	DBG("ft5x02_ADC_TARGET=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_FILTER_FRAME_NOISE = atoi(value);
	DBG("ft5x02_FILTER_FRAME_NOISE=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_POWERNOISE_FILTER_TH= atoi(value);
	DBG("ft5x02_POWERNOISE_FILTER_TH=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_KX_LR= atoi(value);
	DBG("ft5x02_KX_LR=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_KY_UD= atoi(value);
	DBG("ft5x02_KY_UD=%s\n", value);

	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	
	g_param_ft5x02.ft5x02_ESD_FILTER_FRAME = atoi(value);
	DBG("ft5x02_ESD_FILTER_FRAME=%s\n", value);

/*********************************************************************/	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_tx_num = atoi(value);
	DBG("ft5x02_tx_num=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_rx_num = atoi(value);
	DBG("ft5x02_rx_num=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_gain = atoi(value);
	DBG("ft5x02_gain=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_voltage = atoi(value);
	DBG("ft5x02_voltage=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_scanselect = atoi(value);
	DBG("ft5x02_scanselect=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	for(j = 0; j < g_ft5x02_tx_num; j++) {
		char * psrc = value;
		g_ft5x02_tx_order[j] = atoi(ft5x02_sub_str(psrc, j+1));
	}
	DBG("ft5x02_tx_order=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	g_ft5x02_tx_offset = atoi(value);
	DBG("ft5x02_tx_offset=%s\n", value);
	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	for(j = 0; j < g_ft5x02_tx_num; j++) {
		char * psrc = value;
		g_ft5x02_tx_cap[j] = atoi(ft5x02_sub_str(psrc, j+1));
	}
	DBG("ft5x02_tx_cap=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	for(j = 0; j < g_ft5x02_rx_num; j++) {
		char * psrc = value;
		g_ft5x02_rx_order[j] = atoi(ft5x02_sub_str(psrc, j+1));
	}
	DBG("ft5x02_rx_order=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	for(j = 0; j < g_ft5x02_rx_num/2; j++) {
		char * psrc = value;
		g_ft5x02_rx_offset[j] = atoi(ft5x02_sub_str(psrc, j+1));
	}
	DBG("ft5x02_rx_offset=%s\n", value);

	
	sprintf(key, "%s", String_Param_FT5X02[i]);
	if (ini_get_key(filedata,section,key,value)<0)
		goto ERROR_RETURN;
	i++;
	for(j = 0; j < g_ft5x02_rx_num; j++) {
		char * psrc = value;
		g_ft5x02_rx_cap[j] = atoi(ft5x02_sub_str(psrc, j+1));
	}
	DBG("ft5x02_rx_cap=%s\n", value);
	
	if (filedata) 
		kfree(filedata);
	return 0;
ERROR_RETURN:
	if (filedata) 
		kfree(filedata);
	return -1;
}

