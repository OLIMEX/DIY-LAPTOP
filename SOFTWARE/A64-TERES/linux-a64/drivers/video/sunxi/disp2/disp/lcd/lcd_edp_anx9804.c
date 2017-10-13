#include "lcd_edp_anx9804.h"
#include "../disp_sys_intf.h"
#include <asm/io.h>

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);
#define panel_reset(val) sunxi_lcd_gpio_set_value(sel, 0, val)
#define panel_lcden(val) sunxi_lcd_gpio_set_value(sel, 1, val)

#define LCD_GPIO_SDA 1
#define LCD_GPIO_SCL 2

#define	LCD_SDA_INDEX	1
#define	LCD_SCK_INDEX	0

#define	LCD_SDA_INDEX_CFG1	(1)
#define	LCD_SCK_INDEX_CFG1	(0)

#define tick_printf printk

void __iomem *base_addr;
inline void IIC_SCLB_LOW(void)
{
#if 0
	sunxi_lcd_gpio_set_direction(0, LCD_GPIO_SCL, 1);
	sunxi_lcd_gpio_set_value(0, LCD_GPIO_SCL, 0);
#else
	//	printf("!!!!!!%s\n",__func__);
	uint reg_val;

    // Make GPIO
	reg_val = readl(base_addr + 0x24*7);
	reg_val &= ~(0x7 << (LCD_SCK_INDEX_CFG1*4));
	reg_val |= (0x1 << (LCD_SCK_INDEX_CFG1*4));
	writel(reg_val, (base_addr + 0x24*7));

    // Pull low
	reg_val = readl(base_addr + 0x24*7 + 0x10); 
	reg_val &= ~(1<<LCD_SCK_INDEX);
	writel(reg_val, (base_addr + 0x24*7 + 0x10));

#endif	
}

inline void IIC_SCLB_HIGH(void)
{
    #if 0
	sunxi_lcd_gpio_set_direction(0, LCD_GPIO_SCL, 0);

#else
	//	printf("!!!!!!%s\n",__func__);
	uint reg_val;
	reg_val = readl(base_addr + 0x24*7);	//PH4 CFG
	reg_val &= ~(0x7 << (LCD_SCK_INDEX_CFG1*4));
	writel(reg_val, (base_addr + 0x24*7));

#endif	
}

inline void IIC_SDAB_LOW(void)
{
    #if 0
	sunxi_lcd_gpio_set_direction(0, LCD_GPIO_SDA, 1);
	sunxi_lcd_gpio_set_value(0, LCD_GPIO_SDA, 0);

#else
	//printf("!!!!!!%s\n",__func__);
	uint reg_val;

    //Make gpio
	reg_val = readl(base_addr + 0x24*7);
	reg_val &= ~(0x7 << (LCD_SDA_INDEX_CFG1*4));
	reg_val |= (0x1 << (LCD_SDA_INDEX_CFG1*4));
	writel(reg_val, (base_addr + 0x24*7));

    // Pull low
	reg_val = readl(base_addr + 0x24*7 + 0x10);	
	reg_val &= ~(1<<LCD_SDA_INDEX);
	writel(reg_val, (base_addr + 0x24*7 + 0x10));
#endif
}

inline void IIC_SDAB_HIGH(void)
{
    #if 0

	sunxi_lcd_gpio_set_direction(0, LCD_GPIO_SDA, 0);

#else
	//printf("!!!!!!%s\n",__func__);
	uint reg_val;
	reg_val = readl(base_addr + 0x24*7);
	reg_val &= ~(0x7 << (LCD_SDA_INDEX_CFG1*4));
	writel(reg_val, (base_addr + 0x24*7));

#endif	
}

inline __u32 CHECK_SDAB_HIGH(void)
{
    #if 0	

	sunxi_lcd_gpio_set_direction(0, LCD_GPIO_SDA, 0);
	return sunxi_lcd_gpio_set_value(0, LCD_GPIO_SDA, 2);//if data==2 ,return value 

#else
	//	printf("!!!!!!%s\n",__func__);
	uint reg_val;
	
	reg_val = readl(base_addr + 0x24*7);	//PH5 CFG
	reg_val &= ~(0x7 << (LCD_SDA_INDEX_CFG1*4));
	//reg_val |= (0x1 << 12);	//SDA
	writel(reg_val, (base_addr + 0x24*7));
	
	reg_val = readl(base_addr + 0x24*7 + 0x10);
	reg_val = (reg_val>>LCD_SDA_INDEX)&1;
	return reg_val;
#endif	
}
    
int i2cB_clock( void )
{
	int sample = 0;
    
	sunxi_lcd_delay_us(5); 
	IIC_SCLB_HIGH();
	sunxi_lcd_delay_us(5); 
	IIC_SCLB_LOW();
	return (sample);
}

int i2cB_ack(void)
{
	sunxi_lcd_delay_us(5);
	IIC_SCLB_HIGH();
	sunxi_lcd_delay_us(5);
	if(CHECK_SDAB_HIGH())
	{
		sunxi_lcd_delay_us(5); 
		IIC_SCLB_LOW();
		sunxi_lcd_delay_us(5); 
		IIC_SDAB_HIGH();
		sunxi_lcd_delay_us(5); 
		return(1);
	}
	else
	{
//		sunxi_lcd_delay_us(5);
		IIC_SCLB_LOW();
		sunxi_lcd_delay_us(5);
		IIC_SDAB_HIGH();
		return(0);
	}
}

//---------------------------------------------------------
void i2cBStartA( void )
{
	IIC_SCLB_HIGH();
	IIC_SDAB_HIGH();
	sunxi_lcd_delay_us(5); 
	IIC_SDAB_LOW();
	sunxi_lcd_delay_us(5); 
	IIC_SCLB_LOW();
}

int i2cBStart( void )
{
	IIC_SDAB_HIGH();
	IIC_SCLB_HIGH();
	sunxi_lcd_delay_us(5); 
	if(CHECK_SDAB_HIGH())
	{
		{
			i2cBStartA();
			return(1);
		}
	}
	return(0);
}


void i2cBStop(void)
{
   IIC_SDAB_LOW();
   sunxi_lcd_delay_us(5); 
   IIC_SCLB_HIGH();
   sunxi_lcd_delay_us(5); 
   IIC_SDAB_HIGH();
   sunxi_lcd_delay_us(5); 
}
//---------------------------------------------------------
int i2cBTransmit(__u8 value)
{
	register __u8 i ;
	for ( i=0 ; i<8 ; i++ )
	{
		if((value&0x80)==0x80)
		{
			IIC_SDAB_HIGH();
		}
		else
		{
			IIC_SDAB_LOW();
		}
		value = value << 1;
		i2cB_clock();
	}
	return(!i2cB_ack());
}

int i2cBLocateSubAddr(__u8 slave_addr, __u8 sub_addr)
{
//	register __u8 i;
//	for (i=0; i<3; i++)
	{
		//Start I2C
		if (i2cBStart())
		{
			//Slave address
			if (i2cBTransmit(slave_addr))
			{
				if (i2cBTransmit(sub_addr))
					return(1);
			}
		}
		i2cBStop();
	}
	return(0);
}

//---------------------------------------------------------
int i2cBReceive(__u8* value)
{
	register __u8 i ;
	*value = 0;
	for ( i=0 ; i<8 ; i++ )
	{
		IIC_SCLB_HIGH();
		sunxi_lcd_delay_us(5); 
		if(CHECK_SDAB_HIGH())
		{
			*value |= (1<<(7-i));
		}
		IIC_SCLB_LOW();
		sunxi_lcd_delay_us(5);
	}
	IIC_SDAB_HIGH();
	IIC_SCLB_HIGH();
	sunxi_lcd_delay_us(5);
	IIC_SCLB_LOW();
	IIC_SDAB_HIGH();
	sunxi_lcd_delay_us(5);
	return(1);
}

int i2cBLocateSubDataR(__u8 slave_addr, __u8* value)
{
	register __u8 i;
	for (i=0; i<3; i++)
	{
		//Start I2C
		if (i2cBStart())
		{
			//Slave address
			if (i2cBTransmit(slave_addr|1))
			{
				if (i2cBReceive(value))
					return(1);
			}
		}
		i2cBStop();
	}
	return(0);
}
//---------------------------------------------------------
static __s32 lcd_iic_write(__u8 slave_addr, __u8 sub_addr, __u8 value)
{
	if (i2cBLocateSubAddr(slave_addr, sub_addr))
	{
		//value
		if (i2cBTransmit(value))
		{
			i2cBStop();
			return(1);
		}
	}
	i2cBStop();
	return(0);
}


 static __s32 lcd_iic_read(__u8 slave_addr, __u8 sub_addr, __u8* value)
{
	if (i2cBLocateSubAddr(slave_addr, sub_addr))
		i2cBStop();
	sunxi_lcd_delay_us(10);
	if (i2cBLocateSubDataR(slave_addr,value))
		i2cBStop();
	return(1);
}

static void LCD_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, LCD_power_on, 20);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);  //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 150);   //open lcd power, than delay 200ms	
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(u32 sel)
{
//	printk("================power on \n");
	panel_reset(0);

	sunxi_lcd_power_enable(sel, 1);//config lcd_power pin to open lcd power1
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_enable(sel, 2);//config lcd_power pin to open lcd power2
	sunxi_lcd_delay_ms(10);
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power
	sunxi_lcd_delay_ms(5);
	panel_reset(1);
	sunxi_lcd_delay_ms(10);
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_delay_ms(20);
	panel_reset(0);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 2);//config lcd_power pin to close lcd power2
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 1);//config lcd_power pin to close lcd power1
}

static void LCD_bl_open(u32 sel)
{
#if 0
		sunxi_lcd_pin_cfg(sel, 0);
		sunxi_lcd_power_disable(sel, 0);
		sunxi_lcd_delay_ms(100);
		sunxi_lcd_pin_cfg(sel, 1);
		sunxi_lcd_delay_ms(10);
		sunxi_lcd_power_enable(sel, 0);
		sunxi_lcd_delay_ms(200);
#endif
	sunxi_lcd_pwm_enable(sel);
    printk("==================bl open\n");
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
    panel_lcden(1);
}

static void LCD_bl_close(u32 sel)
{
    panel_lcden(0);
    printk("==================bl close\n");
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
}

static void LCD_panel_init(u32 sel)
{
    __u8 c;
    __s32 i;
    __u32 count = 0;
    int lanes;
    __u32 data_rate;
    int colordepth;
    int lcd_edp_tx_rate=0;
    int lcd_model_name;
		base_addr=ioremap(0x01C20800,  0x400);
//   disp_sys_script_get_item("lcd0", "lcd_edp_tx_rate", &lcd_edp_tx_rate, 1);
//   disp_sys_script_get_item("lcd0", "lcd_edp_tx_lane", &lanes, 1);
//   disp_sys_script_get_item("lcd0", "lcd_edp_colordepth", &colordepth, 1);
//   disp_sys_script_get_item("lcd0", "lcd_model_name", &lcd_model_name, 1);
   lcd_edp_tx_rate=2;
   lanes=1;
   colordepth=1;
   lcd_model_name=2;   
   tick_printf("\nlcd_edp_tx_rate=%d,\nlcd_edp_tx_lane=%d,\nlcd_edp_colordepth=%d,\nlcd_model_name=%d\n",lcd_edp_tx_rate,lanes,colordepth,lcd_model_name);

		if(lcd_model_name==1){
				//lcd_edp_tx_rate =1;
				tick_printf("anx9804_panel LCD_panel_init======================\n");
				//lanes = info->lcd_edp_tx_lane;
			    data_rate = 0x06;
			    if(lcd_edp_tx_rate == 1)
			    {
			        data_rate = 0x06;//1.62G
			    }
			    else if(lcd_edp_tx_rate == 2)
			    {
			        data_rate = 0x0a;//2.7G
			    }
			
			   // colordepth = (info->lcd_edp_colordepth == 1)? 0x00:0x10;//0x00: 6bit;  0x10:8bit
			   colordepth = (colordepth == 1)? 0x00:0x10;//0x00: 6bit;  0x10:8bit
			
			
			     //HW reset
				lcd_iic_write(0x72, DP_TX_RST_CTRL_REG, DP_TX_RST_HW_RST);
				sunxi_lcd_delay_ms(10);
				lcd_iic_write(0x72, DP_TX_RST_CTRL_REG, 0x00);
				//Power on total and select DP mode
			  lcd_iic_write(0x72, DP_POWERD_CTRL_REG, 0x00 );
				
				//get chip ID. Make sure I2C is OK
				lcd_iic_read(0x72, DP_TX_DEV_IDH_REG , &c);
				if(c==0x98)
				{
				//	printk("ANX9804 Chip found\n")
				tick_printf("NX9804 Chip found\n");
				}
				else
				{
				//	printk("ANX9804 Chip not found\n");
				tick_printf("NX9804 Chip not found\n");
				}
			
			#if 0
				//for clock detect
				for(i=0;i<50;i++)
				{
					lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
					lcd_iic_write(0x70, DP_TX_SYS_CTRL1_REG, c);
					lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
					if((c&DP_TX_SYS_CTRL1_DET_STA)!=0)
					{
				//		printk("ANX9804 clock is detected.\n");
						break;
					}
			
					sunxi_lcd_delay_ms(10);
				}
			#endif
			
			       //check whether clock is stable
				for(i=0;i<50;i++)
				{
					lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
					lcd_iic_write(0x70, DP_TX_SYS_CTRL2_REG, c);
					lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
					if((c&DP_TX_SYS_CTRL2_CHA_STA)==0)
					{
				//		printk("ANX9804 clock is stable.\n");
						break;
					}
					sunxi_lcd_delay_ms(10);
				}
			
				//VESA range, 8bits BPC, RGB 
				lcd_iic_write(0x72, DP_TX_VID_CTRL2_REG, colordepth);
				
				//ANX9804 chip analog setting
				lcd_iic_write(0x70, DP_TX_PLL_CTRL_REG, 0x07); 
				lcd_iic_write(0x72, DP_TX_PLL_FILTER_CTRL3, 0x19); 
				lcd_iic_write(0x72, DP_TX_PLL_CTRL3, 0xd9); 
				
				//lcd_iic_write(0x7a, 0x38, 0x10); 
				//lcd_iic_write(0x7a, 0x39, 0x20); 
				//lcd_iic_write(0x7a, 0x65, 0x00); 
				
				//Select AC mode
				lcd_iic_write(0x72, DP_TX_RST_CTRL2_REG, 0x40); 
				
				//lcd_iic_write(0x7a, 0x61, 0x10); 
				//lcd_iic_write(0x7a, 0x62, 0x10); 
				//lcd_iic_write(0x7a, 0x63, 0x10); 
				//lcd_iic_write(0x7a, 0x64, 0x10); 
			
				//ANX9804 chip analog setting
				lcd_iic_write(0x72, ANALOG_DEBUG_REG1, 0xf0);
				lcd_iic_write(0x72, ANALOG_DEBUG_REG3, 0x99);
				lcd_iic_write(0x72, DP_TX_PLL_FILTER_CTRL1, 0x7b);
				lcd_iic_write(0x70, DP_TX_LINK_DEBUG_REG, 0x30);
				lcd_iic_write(0x72, DP_TX_PLL_FILTER_CTRL, 0x06);
			
				//force HPD
				lcd_iic_write(0x70, DP_TX_SYS_CTRL3_REG, 0x30);
				//power on 4 lanes
				lcd_iic_write(0x70, 0xc8, 0x00);
				//lanes setting
				lcd_iic_write(0x70, 0xa3, 0x00);
				lcd_iic_write(0x70, 0xa4, 0x00);
				lcd_iic_write(0x70, 0xa5, 0x00);
				lcd_iic_write(0x70, 0xa6, 0x00);
			
			#if 0
				//step 1: read DPCD 0x00001, the correct value should be 0x0a, or 0x06
				lcd_iic_write(0x70,  0xE4,  0x80);
			
				//set read cmd and count, read 2 __u8s data, get downstream max_bandwidth and max_lanes
				lcd_iic_write(0x70, 0xE5,  0x19);
			
				//set aux address19:0
				lcd_iic_write(0x70,  0xE6,  0x01);
				lcd_iic_write(0x70,  0xE7,  0x00);
				lcd_iic_write(0x70,  0xE8,  0x00);
			
				//Enable Aux
				lcd_iic_write(0x70,  0xE9, 0x01);
			
				//wait aux finished
				for(i=0; i<50; i++)
				{
				  lcd_iic_read(0x70,  0xE9,  &c);
				  if(c==0x00)
				  {
				    break;
				  }
				}
			
				//read data from buffer
				lcd_iic_write(  0x70,  0xF0,   &max_bandwidth);
				lcd_iic_write(  0x70,  0xF1,   &max_lanes);
				debug_pr__s32f("max_bandwidth = %.2x, max_lanes = %.2x\n", (WORD)max_bandwidth, (WORD)max_lanes);
			#endif
			
				//reset AUX CH
				lcd_iic_write(0x72,  DP_TX_RST_CTRL2_REG,  0x44);
				lcd_iic_write(0x72,  DP_TX_RST_CTRL2_REG,  0x40);
			
				//to save power
				lcd_iic_write(0x72, DP_POWERD_CTRL_REG, 0x10 );//audio power down
				lcd_iic_write(0x70, DP_TX_HDCP_CONTROL_0_REG, 0x00 );
				lcd_iic_write(0x70, 0xA7, 0x00 );//Spread spectrum 30 kHz
				//end
			
			  /* enable ssc function */
			  lcd_iic_write(0x70, 0xa7, 0x00);                   // disable SSC first
			  lcd_iic_write(0x70, 0xa0, 0x00);                   //disable speed first
			  lcd_iic_write(0x72, 0xde, 0x99);                   //set duty cycle
			  lcd_iic_read(0x70, 0xc7, &c);                      //reset DP PLL
			  lcd_iic_write(0x70, 0xc7, c & (~0x40));
			  lcd_iic_read(0x70, 0xd8, &c);                      //M value select, select clock with downspreading
			  lcd_iic_write(0x70, 0xd8, (c | 0x01));
			  lcd_iic_write(0x70, 0xc7, 0x02);                   //PLL power 1.7V
			  lcd_iic_write(0x70, 0xd0, 0xb8);                   // ssc d 0.5%
			  lcd_iic_write(0x70, 0xd1, 0x6D);                   // ctrl_th 30.4237K
			  lcd_iic_write(0x70, 0xa7, 0x10);                   // enable SSC
			  lcd_iic_read(0x72, 0x07, &c);                      //ssc reset
			  lcd_iic_write(0x72, 0x07, c | 0x80);
			  lcd_iic_write(0x72, 0x07, c & (~0x80));
			
				//Select 2.7G
				//lcd_iic_write(0x70, DP_TX_LINK_BW_SET_REG, 0x0a);
				lcd_iic_write(0x70, DP_TX_LINK_BW_SET_REG, data_rate);	//0x06: Select 1.62G
			
				//Select 4 lanes
				lcd_iic_write(0x70, DP_TX_LANE_COUNT_SET_REG, lanes);
				
				//strart link traing
				//DP_TX_LINK_TRAINING_CTRL_EN is self clear. If link training is OK, it will self cleared.
				lcd_iic_write(0x70, DP_TX_LINK_TRAINING_CTRL_REG, DP_TX_LINK_TRAINING_CTRL_EN);
				sunxi_lcd_delay_ms(5);
				lcd_iic_read(0x70, DP_TX_LINK_TRAINING_CTRL_REG, &c);
				while((c&0x01)!=0)
				{
			//		printk("ANX9804 Waiting...\n");
					sunxi_lcd_delay_ms(5);
			        count ++;
			        if(count > 100)
			        {
			    //        printk("ANX9804 Link training fail\n");
			            break;
			        }
					lcd_iic_read(0x70, DP_TX_LINK_TRAINING_CTRL_REG, &c);
				}
				//lcd_iic_write(0x7a, 0x7c, 0x02);  	
			
			    //BIST MODE: video format. In normal mode, don't need to config these reg from 0x12~0x21
				//lcd_iic_write(0x72, 0x12, 0x2c);
				//lcd_iic_write(0x72, 0x13, 0x06);
				//lcd_iic_write(0x72, 0x14, 0x00);
				//lcd_iic_write(0x72, 0x15, 0x06);
				//lcd_iic_write(0x72, 0x16, 0x02);
				//lcd_iic_write(0x72, 0x17, 0x04);
				//lcd_iic_write(0x72, 0x18, 0x26);
				//lcd_iic_write(0x72, 0x19, 0x50);
				//lcd_iic_write(0x72, 0x1a, 0x04);
				//lcd_iic_write(0x72, 0x1b, 0x00);
				//lcd_iic_write(0x72, 0x1c, 0x04);
				//lcd_iic_write(0x72, 0x1d, 0x18);
				//lcd_iic_write(0x72, 0x1e, 0x00);
				//lcd_iic_write(0x72, 0x1f, 0x10);
				//lcd_iic_write(0x72, 0x20, 0x00);
				//lcd_iic_write(0x72, 0x21, 0x28);
			
				//lcd_iic_write(0x72, 0x11, 0x03);
				
			    //enable BIST. In normal mode, don't need to config this reg
				//lcd_iic_write(0x72, 0x0b, 0x08);
				
				//enable video input, set DDR mode, the input DCLK should be 102.5MHz; 
				//In normal mode, set this reg to 0x81, SDR mode, the input DCLK should be 205MHz
				//lcd_iic_write(0x72, 0x08, 0x8d);
				//lcd_iic_write(0x72, 0x08, 0x81);
				lcd_iic_write(0x72, 0x08, 0x81);
				
			    //force HPD and stream valid
				lcd_iic_write(0x70, 0x82, 0x33);
				
				return;
		}else{
		    //==========================9807====================

				//Power on total and select DP mode
				       lcd_iic_write(0x72, DP_POWERD_CTRL_REG, 0x30);
				
					//get chip ID. Make sure I2C is OK
					lcd_iic_read(0x72, 0x01 , &c);
						if(c==0xaa)
					{
						tick_printf("ANX9807 Chip found\n");
					}
					else
					{
						tick_printf("ANX9807 Chip not found\n");
					}
							
					lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
					lcd_iic_write(0x70, DP_TX_SYS_CTRL1_REG, 0x0);
					sunxi_lcd_delay_ms(100);
					lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
					count = 0;
					while((c&DP_TX_SYS_CTRL1_DET_STA)==0)
						{
							tick_printf("wait clock detect.\n");
							lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
							lcd_iic_write(0x70, DP_TX_SYS_CTRL1_REG, 0x0);
							lcd_iic_read(0x70, DP_TX_SYS_CTRL1_REG, &c);
							sunxi_lcd_delay_ms(100);
							count ++;
				        if(count > 10)
				        {
				            tick_printf("ANX9807 clock detect fail\n");
				            break;
				        }
						}
					
					tick_printf("..................................clock is detected.\n");
					sunxi_lcd_delay_ms(10);
				
				
					lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
					lcd_iic_write(0x70, DP_TX_SYS_CTRL2_REG, 0x40);
					sunxi_lcd_delay_ms(10);
					lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
					count = 0;
					while((c&DP_TX_SYS_CTRL2_CHA_STA)==0x04)
						{
							tick_printf("wait clock stable.\n");
							lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
							lcd_iic_write(0x70, DP_TX_SYS_CTRL2_REG, 0x40);
							sunxi_lcd_delay_ms(10);
							lcd_iic_read(0x70, DP_TX_SYS_CTRL2_REG, &c);
							sunxi_lcd_delay_ms(1000);
							count ++;
				        if(count > 50)
				        {
				            tick_printf("ANX9807 clock detect fail\n");
				            break;
				        }
						}
					tick_printf("..................................clock is stable.\n");
					sunxi_lcd_delay_ms(10);
				
				
					//VESA range, 6bits BPC, RGB 
					lcd_iic_write(0x72, DP_TX_VID_CTRL2_REG, 0x00);//0x00: 6bit;  0x10:8bit
					
					//setting pll
					lcd_iic_write(0x70, DP_TX_PLL_CTRL_REG, 0x00);                 
						
					//analog setting
					lcd_iic_write(0x72, ANALOG_DEBUG_REG1, 0xF0);              
				
					//new PRBS7
					lcd_iic_write(0x70, DP_TX_LINK_DEBUG_REG, 0x30);
				
				                                  
					//reset AUX CH
					lcd_iic_write(0x72,  DP_TX_RST_CTRL2_REG,  0x04);                     
					lcd_iic_write(0x72,  DP_TX_RST_CTRL2_REG,  0x00);                      
				
					
				       //Select 2.7G
				       lcd_iic_write(0x70, DP_TX_LINK_BW_SET_REG, 0x0A);//0x0A--2.7  0x06--1.6G
				       //Select 1 lanes
				       lcd_iic_write(0x70, DP_TX_LANE_COUNT_SET_REG, 0x01);          
					
					//strart link traing
					//DP_TX_LINK_TRAINING_CTRL_EN is self clear. If link training is OK, it will self cleared.
					lcd_iic_write(0x70, DP_TX_LINK_TRAINING_CTRL_REG, DP_TX_LINK_TRAINING_CTRL_EN);
					sunxi_lcd_delay_ms(10);
					lcd_iic_read(0x70, DP_TX_LINK_TRAINING_CTRL_REG, &c);
					count = 0;
					while((c&0x80)!=0)                                                                                //UPDATE: FROM 0X01 TO 0X80
					{
						tick_printf("Waiting...\n");
						sunxi_lcd_delay_ms(5);
						lcd_iic_read(0x70, DP_TX_LINK_TRAINING_CTRL_REG, &c);
						count ++;
				        if(count > 50)
				        {
				            tick_printf("ANX9807 training fail\n");
				            break;
				        }
					}
				       // 600mv/3.5db
					lcd_iic_write(0x7a, 0x38, 0x3c);
					
					lcd_iic_write(0x72, 0x11, 0x00);
					lcd_iic_write(0x72, 0x08, 0x82);
				    	 	
				       //force HPD and wait stream valid
					lcd_iic_write(0x70, 0x82, 0x33);
					
					return;
		
		
		}

}

static void LCD_panel_exit(u32 sel)
{
	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t anx9804_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "anx9804_panel",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
