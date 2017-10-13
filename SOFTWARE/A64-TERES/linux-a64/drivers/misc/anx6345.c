/*
AN6345 Module by <hehopmajieh@debian.bg> Mitko Gamishev 2016
***************************************************************
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include "anx6345.h"
#include <linux/debugfs.h>
//BIST Mode
/*#define BIST_MODE 1*/

static const unsigned short normal_i2c[] = {0x39, I2C_CLIENT_END};
static __u32 twi_id = 0;
static int i2c_num = 0;
static const unsigned short i2c_address[3] = {0x39,0x38,0x2d};



int i2c_master_reg8_recv(const struct i2c_client *client, const char reg, char *buf, int count, int scl_rate)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msgs[2];
	int ret;
	char reg_buf = reg;
	
	msgs[0].addr = client->addr;
	msgs[0].flags = client->flags;
	msgs[0].len = 1;
	msgs[0].buf = &reg_buf;

	msgs[1].addr = client->addr;
	msgs[1].flags = client->flags | I2C_M_RD;
	msgs[1].len = count;
	msgs[1].buf = (char *)buf;

	ret = i2c_transfer(adap, msgs, 2);

	return (ret == 2)? count : ret;
}

int i2c_master_reg8_send(const struct i2c_client *client, const char reg, const char *buf, int count, int scl_rate)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int ret;
	char *tx_buf = (char *)kmalloc(count + 1, GFP_KERNEL);
	if(!tx_buf)
		return -ENOMEM;
	tx_buf[0] = reg;
	memcpy(tx_buf+1, buf, count); 

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.len = count + 1;
	msg.buf = (char *)tx_buf;

	ret = i2c_transfer(adap, &msg, 1);
	kfree(tx_buf);
	return (ret == 1) ? count : ret;

}


static int anx6345_detect(struct i2c_client *client, struct i2c_board_info *info)
{
        int ret;

//        dprintk(DEBUG_INIT, "%s enter \n", __func__);

  struct i2c_adapter *adapter = client->adapter;
         int address = client->addr;
         const char *name = NULL;
if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                 return -ENODEV;
 name = "anx6345"; 
 strlcpy(info->type, name, I2C_NAME_SIZE);
}

static int anx6345_i2c_read_p0_reg(struct i2c_client *client, char reg, char *val)
{
	int ret;
	client->addr = DP_TX_PORT0_ADDR >> 1;
	ret = i2c_master_reg8_recv(client, reg, val, 1, ANX6345_SCL_RATE) > 0? 0: -EINVAL;
	if(ret < 0)
	{
		printk(KERN_ERR "%s>>err\n",__func__);
	}

	return ret;
}
static int  anx6345_i2c_write_p0_reg(struct i2c_client *client, char reg, char *val)
{
	int ret;
	client->addr = DP_TX_PORT0_ADDR >> 1;
	ret = i2c_master_reg8_send(client, reg, val, 1, ANX6345_SCL_RATE) > 0? 0: -EINVAL;
	if(ret < 0)
	{
		printk(KERN_ERR "%s>>err\n",__func__);
	}

	return ret;
}
static int anx6345_i2c_read_p1_reg(struct i2c_client *client, char reg, char *val)
{
	int ret;
	client->addr = HDMI_TX_PORT0_ADDR >> 1;
	ret = i2c_master_reg8_recv(client, reg, val, 1, ANX6345_SCL_RATE) > 0? 0: -EINVAL;
	if(ret < 0)
	{
		printk(KERN_ERR "%s>>err\n",__func__);
		printk("addr %d",client->addr);
	}

	return ret;
}

static int anx6345_i2c_write_p1_reg(struct i2c_client *client, char reg, char *val)
{
	int ret;
	client->addr = HDMI_TX_PORT0_ADDR >> 1;
	ret = i2c_master_reg8_send(client, reg, val, 1, ANX6345_SCL_RATE) > 0? 0: -EINVAL;
	if(ret < 0)
	{
		printk(KERN_ERR "%s>>err\n",__func__);
	}

	return ret;
}

static int edp_reg_show(struct seq_file *s, void *v)
{
	int i = 0;
	char val;
	struct edp_anx6345 *anx6345 = s->private;
	if(!anx6345)
	{
		printk(KERN_ERR "no edp device!\n");
		return 0;
	}

	seq_printf(s,"0x70:\n");
	for(i=0;i< MAX_REG;i++)
	{
		anx6345_i2c_read_p0_reg(anx6345->client, i , &val);
		seq_printf(s,"0x%02x>>0x%02x\n",i,val);
	}

	
	seq_printf(s,"\n0x72:\n");
	for(i=0;i< MAX_REG;i++)
	{
		anx6345_i2c_read_p1_reg(anx6345->client, i , &val);
		seq_printf(s,"0x%02x>>0x%02x\n",i,val);
	}
	return 0;
}

static int edp_reg_open(struct inode *inode, struct file *file)
{
	struct edp_anx6345 *anx6345 = inode->i_private;
	return single_open(file, edp_reg_show, anx6345);
}

static const struct file_operations edp_reg_fops = {
	.owner		= THIS_MODULE,
	.open		= edp_reg_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


//get chip ID. Make sure I2C is OK
static int get_dp_chip_id(struct i2c_client *client)
{
	char c1,c2;
	int id;
	anx6345_i2c_read_p1_reg(client,SP_TX_DEV_IDL_REG,&c1);
    	anx6345_i2c_read_p1_reg(client,SP_TX_DEV_IDH_REG,&c2);
	id = c2;
	return (id<<8)|c1;
}


#if 0
static int anx980x_aux_rst(struct i2c_client *client)
{
	char val;
	anx6345_i2c_read_p1_reg(client, DP_TX_RST_CTRL2_REG, &val);
	val |= DP_TX_AUX_RST;
    	anx6345_i2c_write_p1_reg(client, DP_TX_RST_CTRL2_REG, &val);
	val &= ~DP_TX_AUX_RST;
    	anx6345_i2c_write_p1_reg(client, DP_TX_RST_CTRL2_REG, &val);
	return 0;
}


static int anx980x_wait_aux_finished(struct i2c_client *client)
{
	char val,cnt;
	cnt = 0;
	
	anx6345_i2c_read_p0_reg(client,DP_TX_AUX_CTRL_REG2, &val);
	while(val&0x01)
	{
		//delay_ms(20);
		cnt ++;
		if(cnt == 10)
		{
		   printk("aux break");
		    anx980x_aux_rst(client);
		    //cnt = 0;
		    break;
		}
		anx6345_i2c_read_p0_reg(client, DP_TX_AUX_CTRL_REG2, &val);
	}

	return 0;
}

static int anx980x_aux_dpcdread_bytes(struct i2c_client *client,unsigned long addr, char cCount,char* pBuf)
{
	char val,i;
	
	val = 0x80;
	anx6345_i2c_write_p0_reg(client, DP_TX_BUF_DATA_COUNT_REG, &val);

	//set read cmd and count
	val = (((char)(cCount-1) <<4)&(0xf0))|0x09;
	anx6345_i2c_write_p0_reg(client, DP_TX_AUX_CTRL_REG, &val);

	//set aux address15:0
	val = (char)addr&0xff;
	anx6345_i2c_write_p0_reg(client, DP_TX_AUX_ADDR_7_0_REG, &val);
	val = (char)((addr>>8)&0xff);
	anx6345_i2c_write_p0_reg(client, DP_TX_AUX_ADDR_15_8_REG, &val);

	//set address19:16 and enable aux
	anx6345_i2c_read_p0_reg(client, DP_TX_AUX_ADDR_19_16_REG, &val);
	val &=(0xf0)|(char)((addr>>16)&0xff);
	anx6345_i2c_write_p0_reg(client, DP_TX_AUX_ADDR_19_16_REG, &val);

	//Enable Aux
	anx6345_i2c_read_p0_reg(client, DP_TX_AUX_CTRL_REG2, &val);
	val |= 0x01;
	anx6345_i2c_write_p0_reg(client, DP_TX_AUX_CTRL_REG2, &val);

	//delay_ms(2);
	anx980x_wait_aux_finished(client);

	for(i =0;i<cCount;i++)
	{
		anx6345_i2c_read_p0_reg(client, DP_TX_BUF_DATA_0_REG+i, &val);

		//printk("c = %.2x\n",(WORD)c);
		*(pBuf+i) = val;

		if(i >= MAX_BUF_CNT)
			return 1;
			//break;
	}

	return 0;
	

}

static int anx_video_map_config(struct i2c_client *client)
{
	char val = 0;
 	char i = 0;
	anx6345_i2c_write_p1_reg(client,  0x40, &val);
	anx6345_i2c_write_p1_reg(client,  0x41, &val);
	anx6345_i2c_write_p1_reg(client,  0x48, &val);
	anx6345_i2c_write_p1_reg(client,  0x49, &val);
	anx6345_i2c_write_p1_reg(client,  0x50, &val);
	anx6345_i2c_write_p1_reg(client,  0x51, &val);
	for(i=0; i<6; i++)
	{    
		val = i;
		anx6345_i2c_write_p1_reg(client,  0x42+i, &val);
	}

	for(i=0; i<6; i++)
	{    
		val = 6+i;
		anx6345_i2c_write_p1_reg(client,  0x4a+i, &val);
	}

	for(i=0; i<6; i++)
	{    
		val = 0x0c+i;
		anx6345_i2c_write_p1_reg(client,  0x52+i, &val);
	}

	return 0;
			
}
static int anx980x_eanble_video_input(struct i2c_client *client)
{
	char val;

	anx6345_i2c_read_p1_reg(client,  DP_TX_VID_CTRL1_REG, &val);
	val |= DP_TX_VID_CTRL1_VID_EN;
	anx6345_i2c_write_p1_reg(client,  DP_TX_VID_CTRL1_REG, &val);
	
	anx_video_map_config(client);
	
	return 0;
}
#endif
#if defined(BIST_MODE)
static int anx6345_bist_mode(struct i2c_client *client)
{
	char val = 0x00;
	//these register are for bist mode
	val = 0x28;
	anx6345_i2c_write_p1_reg(client,SP_TX_TOTAL_LINEL_REG,&val);
	val = 0x03;
	anx6345_i2c_write_p1_reg(client,SP_TX_TOTAL_LINEH_REG,&val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client,SP_TX_ACT_LINEL_REG,&val);
	val = 0x03;
	anx6345_i2c_write_p1_reg(client,SP_TX_ACT_LINEH_REG,&val);
	val = 0x14;
	anx6345_i2c_write_p1_reg(client,SP_TX_VF_PORCH_REG,&val);
	val = 0x04	;
	anx6345_i2c_write_p1_reg(client,SP_TX_VSYNC_CFG_REG,&val);
	val = 0x10;
	anx6345_i2c_write_p1_reg(client,SP_TX_VB_PORCH_REG,&val);
	val = 0x38;
	anx6345_i2c_write_p1_reg(client,SP_TX_TOTAL_PIXELL_REG,&val);
	val = 0x06;
	anx6345_i2c_write_p1_reg(client,SP_TX_TOTAL_PIXELH_REG,&val);
	val = 0x56;
	anx6345_i2c_write_p1_reg(client,SP_TX_ACT_PIXELL_REG,&val);
	val = 0x05;
	anx6345_i2c_write_p1_reg(client,SP_TX_ACT_PIXELH_REG,&val);
	val = 0x31;
	anx6345_i2c_write_p1_reg(client,SP_TX_HF_PORCHL_REG,&val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client,SP_TX_HF_PORCHH_REG,&val);
	val = 0x10;
	anx6345_i2c_write_p1_reg(client,SP_TX_HSYNC_CFGL_REG,&val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client,SP_TX_HSYNC_CFGH_REG,&val);
	val = 0xA1;
	anx6345_i2c_write_p1_reg(client,SP_TX_HB_PORCHL_REG,&val);
	val=0x00;
	anx6345_i2c_write_p1_reg(client,SP_TX_HB_PORCHH_REG,&val);
	val = 0x13;
	anx6345_i2c_write_p1_reg(client,SP_TX_VID_CTRL10_REG,&val);
       //enable BIST. In normal mode, don't need to config this reg
	val = 0x08;
	anx6345_i2c_write_p1_reg(client, 0x0b, &val);
	val=0x81;
	anx6345_i2c_write_p1_reg(client,0x08,&val);
	val=0x33;
	anx6345_i2c_write_p0_reg(client,0x82,&val);
	printk("anx6345 enter bist mode\n");

	return 0;
}
#endif
static int anx6345_init(struct i2c_client *client)
{
	char val = 0x00;
	char i = 0;
	char cnt = 50;
	val = 0x30;	

	val = 0x00;
	anx6345_i2c_write_p1_reg(client,0x05,&val);
	anx6345_i2c_read_p1_reg(client,0x01,&val);
	val = 00;
        anx6345_i2c_write_p1_reg(client, SP_POWERD_CTRL_REG, &val);
	val = SP_TX_RST_HW_RST;
	anx6345_i2c_write_p1_reg(client, SP_TX_RST_CTRL_REG, &val);
	msleep(1);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client, SP_TX_RST_CTRL_REG, &val);
	//reset i2c registers
	val = SP_TX_I2C_REG_RST;
	anx6345_i2c_write_p1_reg(client,  SP_TX_RST_CTRL2_REG, &val);
	val = 0x40;
	msleep(50);
	anx6345_i2c_write_p1_reg(client,  SP_TX_RST_CTRL2_REG, &val);
	val = 0x01;
	anx6345_i2c_write_p1_reg(client,  0xd9, &val);
        anx6345_i2c_read_p1_reg(client, SP_POWERD_CTRL_REG,&val);
	val = 0x00;
        anx6345_i2c_write_p1_reg(client, SP_POWERD_CTRL_REG, &val);	
	//get chip ID. Make sure I2C is OK
	anx6345_i2c_read_p1_reg(client, SP_TX_DEV_IDH_REG,&val );
	if (val==0x63)
		printk("Chip found\n");	

	//for clocl detect
	for(i=0;i<100;i++)
		{
		anx6345_i2c_read_p0_reg(client, SP_TX_SYS_CTRL1_REG,&val);
		anx6345_i2c_write_p0_reg(client, SP_TX_SYS_CTRL1_REG, &val);
		anx6345_i2c_read_p0_reg(client, SP_TX_SYS_CTRL1_REG,&val);
		if((val&SP_TX_SYS_CTRL1_DET_STA)!=0)
		{
			printk("clock is detected.\n");
			break;
		}

		msleep(100);
	} 
for(i=0;i<50;i++)
	{
		anx6345_i2c_read_p0_reg(client, SP_TX_SYS_CTRL2_REG,&val);
		anx6345_i2c_write_p0_reg(client, SP_TX_SYS_CTRL2_REG, &val);
		anx6345_i2c_read_p0_reg(client, SP_TX_SYS_CTRL2_REG,&val);
		if((val&SP_TX_SYS_CTRL2_CHA_STA)==0)
		{
			printk("clock is stable. \n");
			break;
		}
		msleep(100);
	}
	//VESA range, 8bits BPC, RGB 
	val = 0x00;//0x10
	anx6345_i2c_write_p1_reg(client, SP_TX_VID_CTRL2_REG, &val);
	//RK_EDP chip analog setting
	val = 0x07;
	anx6345_i2c_write_p0_reg(client, SP_TX_PLL_CTRL_REG, &val); 
	val = 0x19;
	anx6345_i2c_write_p1_reg(client, PLL_FILTER_CTRL3, &val); 
	val = 0xd9;
	anx6345_i2c_write_p1_reg(client, 0xE6, &val);  // not sure ? DP_TX_PLL_CTRL3
//24bit SDR,negedge latch, and wait video stable
	val = 0x01;
	anx6345_i2c_write_p1_reg(client, SP_TX_VID_CTRL1_REG, &val);//72:08 for 9804 SDR, neg edge 05/04/09 extra pxl
	val = 0x19;
	anx6345_i2c_write_p1_reg(client, 0xE1, &val); 
	val = 0xd9;
	anx6345_i2c_write_p1_reg(client, 0xE1, &val);
	
	//Select AC mode
	val = 0x40; //0x40;
	anx6345_i2c_write_p1_reg(client, SP_TX_RST_CTRL2_REG, &val); 
	
	/*
	//Set bist format 1366 768
	val = 0x20;//0x58; //0x20;
	anx6345_i2c_write_p1_reg(client, SP_TX_TOTAL_LINEL_REG, &val);
	val = 0x03;//0x04;//0x03;
	anx6345_i2c_write_p1_reg(client,  SP_TX_TOTAL_LINEH_REG, &val);

	val = 0x00;//0x38;//0x00;
	anx6345_i2c_write_p1_reg(client,  SP_TX_ACT_LINEL_REG, &val);
	val = 0x03;//0x04;//0x03;
	anx6345_i2c_write_p1_reg(client,  SP_TX_ACT_LINEH_REG,&val);
	val = 0x14;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VF_PORCH_REG, &val);
	val = 0x04;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VSYNC_CFG_REG,&val);
	val = 0x10;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VB_PORCH_REG, &val);
	val = 0x38;//0x62;//0x38;
	anx6345_i2c_write_p1_reg(client,  SP_TX_TOTAL_PIXELL_REG, &val);
	val = 0x06;//0x08;//0x06;
	anx6345_i2c_write_p1_reg(client,  SP_TX_TOTAL_PIXELH_REG, &val);
	val = 0x56;//0x80;//0x56;
	anx6345_i2c_write_p1_reg(client,  SP_TX_ACT_PIXELL_REG, &val);
	val = 0x05;//0x07;//0x05;
	anx6345_i2c_write_p1_reg(client,  SP_TX_ACT_PIXELH_REG, &val);

	val = 0x31;
	anx6345_i2c_write_p1_reg(client,  SP_TX_HF_PORCHL_REG, &val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client,  SP_TX_HF_PORCHH_REG, &val);

	val = 0x10;
	anx6345_i2c_write_p1_reg(client,  SP_TX_HSYNC_CFGL_REG,&val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client, SP_TX_HSYNC_CFGH_REG,&val);
	val = 0xa1;
	anx6345_i2c_write_p1_reg(client,  SP_TX_HB_PORCHL_REG, &val);
	val = 0x00;
	anx6345_i2c_write_p1_reg(client,  SP_TX_HB_PORCHH_REG, &val);
	*/


	
	val = 0x00; //0x03;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VID_CTRL10_REG, &val);
	
	//RK_EDP chip analog setting
	val = 0xf0; //0xf0;
	anx6345_i2c_write_p1_reg(client, ANALOG_DEBUG_REG1, &val);
	val = 0x99; //0x99;
	anx6345_i2c_write_p1_reg(client, ANALOG_DEBUG_REG3, &val);//maybe lane level
	val = 0x7b; //0x7b;
	anx6345_i2c_write_p1_reg(client, PLL_FILTER_CTRL1, &val);
	val = 0x70;//0x30;
	anx6345_i2c_write_p1_reg(client, SP_TX_LINK_DEBUG_REG,&val);
	val = 0x06;//0x06;
	anx6345_i2c_write_p1_reg(client,0xE2, &val);
	val = 0x00;//0x06;
	anx6345_i2c_write_p0_reg(client,0xdd, &val);
	
	//force HPD
	val = 0x30;
	anx6345_i2c_write_p0_reg(client, SP_TX_SYS_CTRL3_REG, &val);
	//power on 4 lanes
	val = 0xce;
	anx6345_i2c_write_p0_reg(client,  0xc8, &val);
	//lanes setting
	val = 0xfe;
	anx6345_i2c_write_p0_reg(client,  0xa3, &val);
	val = 0xff;
	anx6345_i2c_write_p0_reg(client,  0xa4, &val);
	anx6345_i2c_write_p0_reg(client,  0xa5, &val);
	anx6345_i2c_write_p0_reg(client,  0xa6, &val);
	//reset AUX CH
	val = 0x44;
	anx6345_i2c_write_p1_reg(client,  SP_TX_RST_CTRL2_REG, &val);
	val = 0x40;
	msleep(50);
	anx6345_i2c_write_p1_reg(client,  SP_TX_RST_CTRL2_REG, &val);
	//Select 1.62G
	val = 0x0a;
	anx6345_i2c_write_p0_reg(client, SP_TX_LINK_BW_SET_REG, &val);
	//Select 4 lanes
	val = 0x01;
	anx6345_i2c_write_p0_reg(client, SP_TX_LANE_COUNT_SET_REG, &val);
	
#if 1
	val = SP_TX_LINK_TRAINING_CTRL_EN;
	anx6345_i2c_write_p0_reg(client, SP_TX_LINK_TRAINING_CTRL_REG, &val);
	msleep(50);
	anx6345_i2c_write_p0_reg(client,SP_TX_LINK_TRAINING_CTRL_REG, &val);
	while((val&0x01)&&(cnt++ < 10))
	{
		printk("Waiting...\n");
		msleep(50);
		anx6345_i2c_read_p0_reg(client,SP_TX_LINK_TRAINING_CTRL_REG, &val );
	}

	if(cnt >= 10)
	{
		printk("HW LT fail\n");
	}
	else
	{
		printk( "HW LT success ...cnt:%d\n",cnt);
	}

#endif

	//enable video input
	val = 0x81;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VID_CTRL1_REG, &val);
	
	//enable BIST
	val = 0x00; //SP_TX_VID_CTRL4_BIST;
	anx6345_i2c_write_p1_reg(client,  SP_TX_VID_CTRL4_REG, &val);	
                // I2CWriteReg8(fd, 0x08, 0x83);          //SDR:0x81;DDR:0x8f
val=0x83;
	   anx6345_i2c_write_p0_reg(client,  0x08, &val);

	//force HPD and stream valid
	val = 0x03;
	anx6345_i2c_write_p0_reg(client,  0x82, &val);
	return 0;
}

static int anx6345_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
  int ret;











        struct edp_anx6345 *anx6345 = NULL;
        int chip_id;
          printk("Probe Enter!!!\n");


        if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
        {
                dev_err(&client->dev, "Must have I2C_FUNC_I2C.\n");
                ret = -ENODEV;
        }
        anx6345 = kzalloc(sizeof(struct edp_anx6345), GFP_KERNEL);
        if (anx6345 == NULL)
        {
                printk("alloc for struct anx6345 fail\n");
                ret = -ENOMEM;
        }


	client->addr  = 0x39;
	 anx6345->client = client;
        anx6345->pdata = client->dev.platform_data;
        i2c_set_clientdata(client,anx6345);
        debugfs_create_file("edp-reg", S_IRUSR,NULL,anx6345,&edp_reg_fops);
        printk("Probe Enter get id!!!\n");
                anx6345->edp_anx_init = anx6345_init;
  	printk("Probe Enter edp_anx_init!!!\n");
        anx6345->edp_anx_init(client);
        printk("edp anx%x probe ok\n",get_dp_chip_id(client));

        return ret;
}

static int  anx6345_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id id_table[] = {
	{"anx6345", 0 },
	{ }
};

static struct i2c_driver anx6345_i2c_driver  = {
	.driver = {
		.name  = "anx6345",
	},
	.probe		= anx6345_i2c_probe,
	.remove     	= anx6345_i2c_remove,
	.id_table	= id_table,

	.detect         = anx6345_detect,
        .address_list   = normal_i2c,

};

MODULE_DEVICE_TABLE(i2c, id_table);

static int  __init anx6345_module_init(void)
{
	int ret = -1;

	ret = i2c_add_driver(&anx6345_i2c_driver);
	printk("anx6345 Init Ret: %d \n",ret);
        return ret;

}

static void  anx6345_module_exit(void)
{
	i2c_del_driver(&anx6345_i2c_driver);
}


module_exit(anx6345_module_exit);
module_init(anx6345_module_init);

MODULE_AUTHOR("Mitko Gamishev <hehopmajieh@debian.bg>");
MODULE_DESCRIPTION("AN6345 RGB<->eDP Transmitter");
MODULE_LICENSE("GPL");
                                                             
