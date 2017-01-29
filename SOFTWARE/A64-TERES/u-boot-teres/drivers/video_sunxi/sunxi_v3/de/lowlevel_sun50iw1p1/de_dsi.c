#include "de_dsi_type.h"
#include "de_dsi.h"

extern s32 disp_delay_us(u32 us);
extern s32 disp_delay_ms(u32 ms);
u32  dsi_bits_per_pixel[4] = {24,24,18,16};

static volatile __de_dsi_dev_t *dsi_dev[2];
__s32 dsi_hs_clk(__u32 sel,__u32 on_off);

s32 dsi_delay_ms(u32 ms)
{
	disp_delay_ms(ms);

	return 0;
}

s32 dsi_delay_us(u32 us)
{
	disp_delay_us(us);

	return 0;
}

__s32 dsi_set_reg_base(__u32 sel, __u32 base)
{
	dsi_dev[sel]=(__de_dsi_dev_t *)(base);

	return 0;
}

__u32 dsi_get_reg_base(__u32 sel)
{
	return (__u32)dsi_dev[sel];
}

__u32 dsi_get_start_delay(__u32 sel)
{
	return 0;
}

__u32 dsi_get_cur_line(__u32 sel)
{
	return 0;
}

__s32 dsi_irq_enable(__u32 sel, __dsi_irq_id_t id)
{
	if(id<32)
	{
		dsi_dev[sel]->dsi_irq_en0.dwval &= ~(1<<id);
	}
	else
	{
		dsi_dev[sel]->dsi_irq_en1.dwval &= ~(1<<(id-32));
	}
	return 0;
}

__s32 dsi_irq_disable(__u32 sel, __dsi_irq_id_t id)
{
	if(id<32)
	{
		dsi_dev[sel]->dsi_irq_en0.dwval |= (1<<id);
	}
	else
	{
		dsi_dev[sel]->dsi_irq_en1.dwval |= (1<<(id-32));
	}
    return 0;
}

static __s32 dsi_irq_disable_all(__u32 sel)
{
	dsi_dev[sel]->dsi_irq_en0.dwval = 0xffffffff;
	dsi_dev[sel]->dsi_irq_en1.dwval = 0xffffffff;
	return 0;
}

__u32 dsi_irq_query(__u32 sel,__dsi_irq_id_t id)
{
	return 0;
}

__s32 dsi_inst_busy(__u32 sel)
{
	return 0;
}

__s32 dsi_start(__u32 sel,__dsi_start_t func)
{
	return 0;
}

__s32 dsi_open(__u32 sel,disp_panel_para * panel)
{
	dsi_dev[sel]->dsi_cmd_ctl.bits.cmd_mode_en		  = 0;
	dsi_dev[sel]->dsi_vid_ctl0.bits.video_mode_en	  = 1;
	dsi_dev[sel]->dsi_cfg1.bits.dpi_src = 1;

	return 0;
}

__s32 dsi_close(__u32 sel)
{
	dsi_dev[sel]->dsi_cfg1.bits.dpi_src = 0;
	//FIXME
	//LCD_delay_fs(sel, 1);
	dsi_delay_ms(16);
	dsi_dev[sel]->dsi_vid_ctl0.bits.video_mode_en	  = 0;
	dsi_dev[sel]->dsi_cmd_ctl.bits.cmd_mode_en		  = 1;
	dsi_hs_clk(sel,0);
	return 0;
}

__s32 dsi_tri_start(__u32 sel)
{
	return 0;
}

 __s32 dsi_dcs_wr(__u32 sel,__u8 cmd,__u8* para_p,__u32 para_num)
{
	__u8 dt,data0,data1;
	__u8 vc = 0;

	switch(para_num)
	{
	case 0:
		dt		= DSI_DT_DCS_WR_P0;
		data0	= cmd;
		data1	= 0;
		break;
	case 1:
		dt		= DSI_DT_DCS_WR_P1;
		data0	= cmd;
		data1 	= *para_p;
		break;
	default:
		dt		= DSI_DT_DCS_LONG_WR;
		data0	= ((para_num+1)>>0) & 0xff;
		data1 	= ((para_num+1)>>8) & 0xff;
		break;
	}

	if(para_num>1)
	{
		__u32 i=0;
		dsi_dev[sel]->dsi_pkg_ctl1.dwval = *(para_p+i+2) << 24 |
									       *(para_p+i+1) << 16 |
									       *(para_p+i+0) <<  8 |
									         cmd		 <<  0 ;
		i+=3;
		while(i<para_num)
		{
			dsi_dev[sel]->dsi_pkg_ctl1.dwval = *(para_p+i+3) << 24 |
											   *(para_p+i+2) << 16 |
											   *(para_p+i+1) <<  8 |
											   *(para_p+i+0) <<  0 ;
			i+=4;
		};
	}

	dsi_dev[sel]->dsi_pkg_ctl0.dwval = (data1<<16 | data0<<8 | vc<<6 | dt);
	dsi_delay_us(20 + para_num);
	return 0;
}

__s32 dsi_dcs_wr_index(__u32 sel,__u8 index)
{

	return 0;
}

__s32 dsi_dcs_wr_data(__u32 sel,__u8 data)
{

	return 0;
}

__s32 dsi_dcs_wr_0para(__u32 sel,__u8 cmd)
{
	__u8 tmp;
	dsi_dcs_wr(0,cmd,&tmp,0);
	return 0;
}

__s32 dsi_dcs_wr_1para(__u32 sel,__u8 cmd,__u8 para)
{
	__u8 tmp = para;
	dsi_dcs_wr(0,cmd,&tmp,1);
	return 0;
}

__s32 dsi_dcs_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2)
{
	__u8 tmp[2];
	tmp[0] = para1;
	tmp[1] = para2;
	dsi_dcs_wr(0,cmd,tmp,2);
	return 0;
}

__s32 dsi_dcs_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3)
{
	__u8 tmp[3];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	dsi_dcs_wr(0,cmd,tmp,3);
	return 0;
}
__s32 dsi_dcs_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4)
{
	__u8 tmp[4];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	tmp[3] = para4;
	dsi_dcs_wr(0,cmd,tmp,4);
	return 0;
}

__s32 dsi_dcs_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5)
{
	__u8 tmp[5];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	tmp[3] = para4;
	tmp[4] = para5;
	dsi_dcs_wr(0,cmd,tmp,5);
	return 0;
}

__s32 dsi_dcs_wr_memory(__u32 sel,__u32* p_data,__u32 length)
{
	__u32 tx_cntr=length;
	__u32 tx_num;
	__u32* tx_p=p_data;
	__u8 para[256];
	__u32 i;
	__u32 start=1;

	while(tx_cntr)
	{
		if(tx_cntr>=83)
			tx_num = 83;
		else
			tx_num = tx_cntr;
		tx_cntr -= tx_num;

		for(i=0;i<tx_num;i++)
		{
			para[i*3+0] = (*tx_p >> 16) & 0xff;
			para[i*3+1] = (*tx_p >> 8)  & 0xff;
			para[i*3+2] = (*tx_p >> 0)  & 0xff;
			tx_p++;
		}

		if(start)
		{
			dsi_dcs_wr(sel,DSI_DCS_WRITE_MEMORY_START,para,tx_num*3);
			start = 0;
		}
		else
		{
			dsi_dcs_wr(sel,DSI_DCS_WRITE_MEMORY_CONTINUE,para,tx_num*3);
		}
	}
	return 0;
}

__s32 dsi_gen_wr(__u32 sel,__u8 cmd,__u8* para_p,__u32 para_num)
{
	__u8 dt,data0,data1;
	switch(para_num)
	{
	case 0:
		dt		= DSI_DT_GEN_WR_P1;
		data0	= cmd;
		data1	= 0;
		break;
	case 1:
		dt		= DSI_DT_GEN_WR_P2;
		data0	= cmd;
		data1 	= *para_p;
		break;
	default:
		dt		= DSI_DT_GEN_LONG_WR;
		data0	= (para_num>>0) & 0xff;
		data1 	= (para_num>>8) & 0xff;
		break;
	}

	if(para_num>1)
	{
		__u32 i=0;
		dsi_dev[sel]->dsi_pkg_ctl1.dwval = *(para_p+i+2) << 24 |
									       *(para_p+i+1) << 16 |
									       *(para_p+i+0) <<  8 |
									         cmd		 <<  0 ;
		i+=3;
		while(i<para_num)
		{
			dsi_dev[sel]->dsi_pkg_ctl1.dwval = *(para_p+i+3) << 24 |
											   *(para_p+i+2) << 16 |
											   *(para_p+i+1) <<  8 |
											   *(para_p+i+0) <<  0 ;
			i+=4;
		};
	}

	dsi_dev[sel]->dsi_pkg_ctl0.dwval = (data1<<16 | data0<<8 | dt);
	dsi_delay_us(20 + para_num);
	return 0;
}

__s32 dsi_gen_wr_0para(__u32 sel,__u8 cmd)
{
	__u8 tmp;
	dsi_gen_wr(sel,cmd,&tmp,0);
	return 0;
}

__s32 dsi_gen_wr_1para(__u32 sel,__u8 cmd,__u8 para)
{
	__u8 tmp = para;
	dsi_gen_wr(sel,cmd,&tmp,1);
	return 0;
}

__s32 dsi_gen_wr_2para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2)
{
	__u8 tmp[2];
	tmp[0] = para1;
	tmp[1] = para2;
	dsi_gen_wr(sel,cmd,tmp,2);
	return 0;
}

__s32 dsi_gen_wr_3para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3)
{
	__u8 tmp[3];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	dsi_gen_wr(sel,cmd,tmp,3);
	return 0;
}
__s32 dsi_gen_wr_4para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4)
{
	__u8 tmp[4];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	tmp[3] = para4;
	dsi_gen_wr(sel,cmd,tmp,4);
	return 0;
}

__s32 dsi_gen_wr_5para(__u32 sel,__u8 cmd,__u8 para1,__u8 para2,__u8 para3,__u8 para4,__u8 para5)
{
	__u8 tmp[5];
	tmp[0] = para1;
	tmp[1] = para2;
	tmp[2] = para3;
	tmp[3] = para4;
	tmp[4] = para5;
	dsi_gen_wr(sel,cmd,tmp,5);
	return 0;
}

__s32 dsi_set_max_ret_size(__u32 sel,__u32 size)
{
	return 0;
}

__s32 dsi_dcs_rd(__u32 sel,__u8 cmd,__u8* para_p,__u32* num_p)
{
	return 0;
}


__s32 dsi_dcs_rd_memory(__u32 sel,__u32* p_data,__u32 length)
{
	return 0;
}


__s32 dsi_hs_clk(__u32 sel,__u32 on_off)
{
	if(on_off)
	{
		if(!dsi_dev[sel]->dsi_phy_status.bits.phy_ck_stop) {
			dsi_delay_ms(5);
			if(!dsi_dev[sel]->dsi_phy_status.bits.phy_ck_stop)
				__wrn("dsi clk enable error\n");
		}

		dsi_dev[sel]->dsi_ctl1.bits.phy_clk_lane_enable 	= 1;
	}
	else
	{
		dsi_dev[sel]->dsi_ctl1.bits.phy_clk_lane_enable 	= 0;
		if(!dsi_dev[sel]->dsi_phy_status.bits.phy_ck_stop) {
			dsi_delay_ms(5);
			if(!dsi_dev[sel]->dsi_phy_status.bits.phy_ck_stop)
				__wrn("dsi clk disable error\n");
		}
	}
	return 0;
}

__s32 dsi_dphy_cfg_0data(__u32 sel,__u32 code)
{
	dsi_delay_us(5);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en	= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = code;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (code << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en		= 0;
	dsi_delay_us(5);
	return 0;
}

__s32 dsi_dphy_cfg_1data(__u32 sel,__u32 code,__u32 data)
{
	dsi_delay_us(5);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en	= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = code;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (code << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en	= 0;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = data;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (data << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(5);
	return 0;
}

__s32 dsi_dphy_cfg_2data(__u32 sel,__u32 code,__u32 data0,__u32 data1)
{
	dsi_delay_us(5);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en		= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = code;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (code << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_en		= 0;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = data0;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (data0 << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(1);
	//dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_din = data1;
	dsi_dev[sel]->dsi_phy_ctl3.dwval = ((dsi_dev[sel]->dsi_phy_ctl3.dwval) & (~0x00ff0000)) | (data1 << 16);
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 1;
	dsi_delay_us(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clk	= 0;
	dsi_delay_us(5);
	return 0;
}

static __s32 dsi_dphy_cfg_hsfrq(__u32 sel,__u32 hs_frq)
{
	__u32 hs_frq_set;
	__u32 hs_frq_tmp;

	hs_frq_tmp = hs_frq;

	if     ( (hs_frq_tmp >=   80) && (hs_frq_tmp <  90) )	hs_frq_set = 0x00;	//b00_0000;
	else if( (hs_frq_tmp >=   90) && (hs_frq_tmp < 100) )  	hs_frq_set = 0x10;	//b01_0000;
	else if( (hs_frq_tmp >=  100) && (hs_frq_tmp < 110) ) 	hs_frq_set = 0x20;	//b10_0000;
	else if( (hs_frq_tmp >=  110) && (hs_frq_tmp < 130) ) 	hs_frq_set = 0x01;	//b00_0001;
	else if( (hs_frq_tmp >=  130) && (hs_frq_tmp < 140) ) 	hs_frq_set = 0x11;	//b01_0001;
	else if( (hs_frq_tmp >=  140) && (hs_frq_tmp < 150) ) 	hs_frq_set = 0x21;	//b10_0001;
	else if( (hs_frq_tmp >=  150) && (hs_frq_tmp < 170) ) 	hs_frq_set = 0x02;	//b00_0010;
	else if( (hs_frq_tmp >=  170) && (hs_frq_tmp < 180) ) 	hs_frq_set = 0x12;	//b01_0010;
	else if( (hs_frq_tmp >=  180) && (hs_frq_tmp < 200) ) 	hs_frq_set = 0x22;	//b10_0010;
	else if( (hs_frq_tmp >=  200) && (hs_frq_tmp < 220) ) 	hs_frq_set = 0x03;	//b00_0011;
	else if( (hs_frq_tmp >=  220) && (hs_frq_tmp < 240) ) 	hs_frq_set = 0x13;	//b01_0011;
	else if( (hs_frq_tmp >=  240) && (hs_frq_tmp < 250) ) 	hs_frq_set = 0x23;	//b10_0011;
	else if( (hs_frq_tmp >=  250) && (hs_frq_tmp < 270) ) 	hs_frq_set = 0x04;	//b00_0100;
	else if( (hs_frq_tmp >=  270) && (hs_frq_tmp < 300) ) 	hs_frq_set = 0x14;	//b01_0100;
	else if( (hs_frq_tmp >=  300) && (hs_frq_tmp < 330) ) 	hs_frq_set = 0x05;	//b00_0101;
	else if( (hs_frq_tmp >=  330) && (hs_frq_tmp < 360) ) 	hs_frq_set = 0x15;	//b01_0101;
	else if( (hs_frq_tmp >=  360) && (hs_frq_tmp < 400) ) 	hs_frq_set = 0x25;	//b10_0101;
	else if( (hs_frq_tmp >=  400) && (hs_frq_tmp < 450) ) 	hs_frq_set = 0x06;	//b00_0110;
	else if( (hs_frq_tmp >=  450) && (hs_frq_tmp < 500) ) 	hs_frq_set = 0x16;	//b01_0110;
	else if( (hs_frq_tmp >=  500) && (hs_frq_tmp < 550) ) 	hs_frq_set = 0x07;	//b00_0111;
	else if( (hs_frq_tmp >=  550) && (hs_frq_tmp < 600) ) 	hs_frq_set = 0x17;	//b01_0111;
	else if( (hs_frq_tmp >=  600) && (hs_frq_tmp < 650) ) 	hs_frq_set = 0x08;	//b00_1000;
	else if( (hs_frq_tmp >=  650) && (hs_frq_tmp < 700) ) 	hs_frq_set = 0x18;	//b01_1000;
	else if( (hs_frq_tmp >=  700) && (hs_frq_tmp < 750) ) 	hs_frq_set = 0x09;	//b00_1001;
	else if( (hs_frq_tmp >=  750) && (hs_frq_tmp < 800) ) 	hs_frq_set = 0x19;	//b01_1001;
	else if( (hs_frq_tmp >=  800) && (hs_frq_tmp < 850) ) 	hs_frq_set = 0x29;	//b10_1001;
	else if( (hs_frq_tmp >=  850) && (hs_frq_tmp < 900) ) 	hs_frq_set = 0x39;	//b11_1001;
	else if( (hs_frq_tmp >=  900) && (hs_frq_tmp < 950) ) 	hs_frq_set = 0x0a;	//b00_1010;
	else if( (hs_frq_tmp >=  950) && (hs_frq_tmp <1000) ) 	hs_frq_set = 0x1a;	//b01_1010;
	else if( (hs_frq_tmp >= 1000) && (hs_frq_tmp <1050) ) 	hs_frq_set = 0x2a;	//b10_1010;
	else if( (hs_frq_tmp >= 1050) && (hs_frq_tmp <1100) ) 	hs_frq_set = 0x3a;	//b11_1010;
	else if( (hs_frq_tmp >= 1100) && (hs_frq_tmp <1150) ) 	hs_frq_set = 0x0b;	//b00_1011;
	else if( (hs_frq_tmp >= 1150) && (hs_frq_tmp <1200) ) 	hs_frq_set = 0x1b;	//b01_1011;
	else if( (hs_frq_tmp >= 1200) && (hs_frq_tmp <1250) ) 	hs_frq_set = 0x2b;	//b10_1011;
	else if( (hs_frq_tmp >= 1250) && (hs_frq_tmp <1300) ) 	hs_frq_set = 0x3b;	//b11_1011;
	else if( (hs_frq_tmp >= 1300) && (hs_frq_tmp <1350) ) 	hs_frq_set = 0x0c;	//b00_1100;
	else if( (hs_frq_tmp >= 1350) && (hs_frq_tmp <1400) ) 	hs_frq_set = 0x1c;	//b01_1100;
	else if( (hs_frq_tmp >= 1400) && (hs_frq_tmp <1450) ) 	hs_frq_set = 0x2c;	//b10_1100;
	else if( (hs_frq_tmp >= 1450) && (hs_frq_tmp <1500) ) 	hs_frq_set = 0x3c;	//b11_1100;
	else													hs_frq_set = 0x00;	//b00_0000;

	dsi_dphy_cfg_1data(sel,0x44,hs_frq_set<<1);
	return 0;
}

static __s32 dsi_dphy_cfg_pll(__u32 sel,__u32 m,__u32 n)
{
	dsi_dphy_cfg_1data(sel,0x19,0x30);
	dsi_dphy_cfg_1data(sel,0x17,n-1);
	dsi_dphy_cfg_1data(sel,0x18,0x7f & (((m-1)>>0) & 0x1f) );
	dsi_dphy_cfg_1data(sel,0x18,0x80 | (((m-1)>>5) & 0x0f) );
	return 0;
}

static __s32 dsi_dphy_cfg(__u32 sel,disp_panel_para * panel)
{
	/*Input sequence:
	1.Set RSTZ = 1'b0.
	2.Set SHUTDOWNZ = 1'b0.
	3.Set TESTCLEAR = 1'b1.
	4.Set MASTERSLAVEZ = 1'b1 (for MASTER) / 1'b0 (for SLAVE).
	5.Set BASEDIRX to the desired values.
	6.Set all REQUEST inputs to zero.
	7.Wait for t > Tmin.
	8.Set RSTZ = 1'b1.
	9.Set SHUTDOWNZ = 1'b1.
	10.Set TESTCLEAR = 1'b0.
	*/

	/*
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clr       	= 1;
	dsi_dev[sel]->dsi_ctl1.bits.phy_rst     			= 0;
	dsi_dev[sel]->dsi_ctl1.bits.phy_en  			= 0;
	delayms(1);//?
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clr       	= 0;

	dsi_dphy_cfg_hsfrq(sel,dclk*dsi_bits_per_pixel[format]/lane);
	dsi_dphy_cfg_pll(sel,dsi_bits_per_pixel[format]*2,lane*2);
	dsi_dphy_cfg_0data(sel,0x00);

	dsi_dev[sel]->dsi_ctl1.bits.phy_rst     			= 1;
	dsi_dev[sel]->dsi_ctl1.bits.phy_en  			= 1;
	*/

	__u32 mode, lane, format, dclk;

	mode	= panel->lcd_dsi_if;
	lane	= panel->lcd_dsi_lane;
	format	= panel->lcd_dsi_format;
	dclk	= panel->lcd_dclk_freq;

	dsi_dev[sel]->dsi_cfg3.bits.reg_rext             	=	0;
	dsi_dev[sel]->dsi_cfg3.bits.reg_snk      			=	2;
	dsi_dev[sel]->dsi_cfg3.bits.reg_rint				=	2;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_cfg3.bits.reg_rext               	=	1;
	dsi_delay_ms(1);

	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clr       	= 1;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_rst     			= 0;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_en  				= 0;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clr       	= 0;
	dsi_delay_ms(1);

	dsi_dphy_cfg_1data(sel, 0x20, 0xcd);
	dsi_dphy_cfg_1data(sel, 0x22, 0x42);

	if(LCD_DSI_IF_VIDEO_MODE==mode || LCD_DSI_IF_BURST_MODE==mode)
	{
		dsi_dphy_cfg_hsfrq(sel,dclk*dsi_bits_per_pixel[format]/lane);
		dsi_dphy_cfg_pll(sel,dsi_bits_per_pixel[format]*2,lane*2);
		dsi_dphy_cfg_0data(sel,0x00);
	}
	else
	{
		__u32 n = dclk*6/30;
		dsi_dphy_cfg_hsfrq(sel,dclk*36);
		dsi_dphy_cfg_pll(sel,6*n,n);
		dsi_dphy_cfg_0data(sel,0x00);
	}

	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_en  			= 1;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_rst     		= 1;
	dsi_delay_ms(1);
	//dsi_dev[sel]->dsi_phy_ctl1.bits.phy_stop_set	= 10;
	dsi_dev[sel]->dsi_phy_ctl1.dwval = ((dsi_dev[sel]->dsi_phy_ctl1.dwval) & (~0xff)) | 10;
	dsi_dev[sel]->dsi_ctl1.bits.phy_lane_num        = lane-1;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_clk_gating		= 1;
	dsi_delay_ms(1);

	return 0;
}


__u32 dsi_io_open(__u32 sel,disp_panel_para * panel)
{
	dsi_dphy_cfg(sel,panel);
	return 0;
}

__u32 dsi_io_close(__u32 sel)
{
	//modify next time
	dsi_dev[sel]->dsi_ctl1.bits.phy_clk_gating			= 0;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_rst     			= 0;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_ctl1.bits.phy_en  				= 0;
	dsi_delay_ms(1);
	dsi_dev[sel]->dsi_phy_ctl3.bits.phy_cfg_clr       	= 0;
	dsi_delay_ms(1);

	return 0;
}

__s32 dsi_clk_enable(__u32 sel, __u32 en)
{
	dsi_hs_clk(sel,en);
	return 0;
}

static __s32 dsi_basic_cfg(__u32 sel,disp_panel_para * panel)
{
	__u32 mode, lane, format, dclk;

	mode	= panel->lcd_dsi_if;
	lane	= panel->lcd_dsi_lane;
	format	= panel->lcd_dsi_format;
	dclk	= panel->lcd_dclk_freq;

	dsi_dev[sel]->dsi_ctl0.bits.module_en 				= 1;
	if(LCD_DSI_IF_VIDEO_MODE==mode || LCD_DSI_IF_BURST_MODE==mode)
	{
		//dsi_dev[sel]->dsi_ctl2.bits.lp_clk_div 	= (dclk*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10);
		dsi_dev[sel]->dsi_ctl2.dwval = ((dclk*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10)) & 0xff;
		dsi_delay_ms(100);
		//dsi_dev[sel]->dsi_to_ctl0.bits.to_clk_div 		= (dclk*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10);
		dsi_dev[sel]->dsi_to_ctl0.dwval 		= ((dclk*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10)) & 0xff;
	}
	else
	{
		//dsi_dev[sel]->dsi_ctl2.bits.lp_clk_div 	= (dclk*6*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10);
		dsi_dev[sel]->dsi_ctl2.dwval = ((dclk*6*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10)) & 0xff;
		dsi_delay_ms(100);
		//dsi_dev[sel]->dsi_to_ctl0.bits.to_clk_div 		= (dclk*6*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10);
		dsi_dev[sel]->dsi_to_ctl0.dwval = ((dclk*6*dsi_bits_per_pixel[format]+4*lane*10)/(8*lane*10)) & 0xff;
	}
	dsi_dev[sel]->dsi_pkg_ctl2.bits.eotp_tx_en			= 0;
	dsi_dev[sel]->dsi_pkg_ctl2.bits.eotp_rx_en			= 0;
	dsi_dev[sel]->dsi_pkg_ctl2.bits.bta_en    			= 0;
	dsi_dev[sel]->dsi_pkg_ctl2.bits.ecc_rx_en 			= 1;
	dsi_dev[sel]->dsi_pkg_ctl2.bits.crc_rx_en 			= 1;
	dsi_dev[sel]->dsi_pkg_ctl2.bits.vid_rx				= 1;

	//dsi_dev[sel]->dsi_vid_ctl2.bits.lpcmd_time_invact 	= 64;
	//dsi_dev[sel]->dsi_vid_ctl2.bits.lpcmd_time_outvact  = 0;
	dsi_dev[sel]->dsi_vid_ctl2.dwval = 0x00400000;


	//dsi_dev[sel]->dsi_phy_ctl1.bits.max_rd_set     		= 50;
	dsi_dev[sel]->dsi_phy_ctl1.dwval = ((dsi_dev[sel]->dsi_phy_ctl1.dwval) & (~0x7fff0000)) | (50 << 16);
	//dsi_dev[sel]->dsi_phy_ctl0.bits.phy_lp2hs_set	  	= 20;
	//dsi_dev[sel]->dsi_phy_ctl0.bits.phy_hs2lp_set	    = 20;
	dsi_dev[sel]->dsi_phy_ctl0.dwval = 0x00140014;

	//dsi_dev[sel]->dsi_to_ctl1.bits.to_hstx_set      	= 0;
	//dsi_dev[sel]->dsi_to_ctl1.bits.to_lprx_set      	= 0;
	dsi_dev[sel]->dsi_to_ctl1.dwval = 0x0;

	dsi_dev[sel]->dsi_cfg0.bits.force_rx_0				= 0;
	dsi_dev[sel]->dsi_cfg0.bits.force_rx_1				= 0;
	dsi_dev[sel]->dsi_cfg0.bits.force_rx_2				= 0;
	dsi_dev[sel]->dsi_cfg0.bits.force_rx_3				= 0;
	dsi_dev[sel]->dsi_cfg0.bits.base_dir				= 0;
	dsi_dev[sel]->dsi_ctl1.bits.phy_clk_lane_enable  	= 0;    	//disable clk hs
	dsi_dev[sel]->dsi_phy_ctl2.bits.phy_ck_tx_ulps_req  = 0;    	//
	dsi_dev[sel]->dsi_phy_ctl2.bits.phy_ck_tx_ulps_exit	= 0;    	//
	dsi_dev[sel]->dsi_phy_ctl2.bits.phy_data_tx_ulps_req= 0;    	//
	dsi_dev[sel]->dsi_phy_ctl2.bits.phy_data_tx_upls_exit= 0;    	//
	dsi_dev[sel]->dsi_phy_ctl2.bits.phy_tx_triger     	= 0;

	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sw_0p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sw_1p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sw_2p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sr_0p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sr_1p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_sr_2p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.dcs_sw_0p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.dcs_sw_1p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.dcs_sr_0p_tx_lp     	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.max_rd_pkg_size_lp  	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.gen_lw_tx_lp        	= 1;
	dsi_dev[sel]->dsi_cmd_ctl.bits.dcs_lw_tx_lp        	= 1;

	dsi_dev[sel]->dsi_cmd_ctl.bits.cmd_mode_en			= 1;
	return 0;
}

static __s32 dsi_packet_cfg(__u32 sel,disp_panel_para * panel)
{
	if(panel->lcd_dsi_if == LCD_DSI_IF_COMMAND_MODE)
	{
		__u32 vc		=	0;
		__u32 format	=	panel->lcd_dsi_format;
		__u32 x			=	panel->lcd_x;

		dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_vid          	= vc;
		switch(format)
		{
		case 0:
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_in_format  	= 11;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_out_format	= 11;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.lut_size_cfg	= 2;
			break;
		case 1:
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_in_format	= 9;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_out_format	= 9;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.lut_size_cfg	= 1;
			break;
		case 2:
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_in_format	= 9;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_out_format	= 9;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.lut_size_cfg	= 1;
			break;
		case 3:
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_in_format  	= 8;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_out_format	= 8;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.lut_size_cfg	= 0;
			break;
		default:
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_in_format  	= 11;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.dbi_out_format	= 11;
			dsi_dev[sel]->dsi_dbi_ctl0.bits.lut_size_cfg	= 2;
			break;
		}

		dsi_dev[sel]->dsi_dbi_ctl0.bits.partion_mode  		= 1;

		dsi_dev[sel]->dsi_dbi_ctl1.bits.allowed_cmd_size 	= x*dsi_bits_per_pixel[format]/8+1;//??
		dsi_dev[sel]->dsi_dbi_ctl1.bits.wr_cmd_size 		= x*dsi_bits_per_pixel[format]/8+1;//??

		dsi_dev[sel]->dsi_pkg_ctl2.bits.bta_en 			= 0;
		dsi_dev[sel]->dsi_cmd_ctl.bits.pkg_ack_req		= 0;
		dsi_dev[sel]->dsi_cmd_ctl.bits.te_ack_en		= 0;

		dsi_dev[sel]->dsi_cmd_ctl.bits.cmd_mode_en		= 1;

		dsi_dev[sel]->dsi_cfg2.bits.dbi_rst			= 1;
		dsi_dev[sel]->dsi_cfg2.bits.dbi_src				= 1;
	}
	else
	{
		__u32 vc		=	0;
		__u32 lane		=	panel->lcd_dsi_lane;
		__u32 format	=	panel->lcd_dsi_format;
		__u32 x			=	panel->lcd_x;
		__u32 y			=	panel->lcd_y;
		__u32 ht		=	panel->lcd_ht;
		__u32 hbp		=	panel->lcd_hbp;
		__u32 hspw		=	panel->lcd_hspw;
		__u32 vt		=	panel->lcd_vt;
		__u32 vbp		=	panel->lcd_vbp;
		__u32 vspw		=	panel->lcd_vspw;

		switch(format)
		{
		case 0:
			dsi_dev[sel]->dsi_cfg1.bits.dpi_src_format  = 0;
			dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_format  = 5;
			break;
		case 1:
			dsi_dev[sel]->dsi_cfg1.bits.dpi_src_format  = 1;
			dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_format  = 3;
			break;
		case 2:
			dsi_dev[sel]->dsi_cfg1.bits.dpi_src_format  = 1;
			dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_format  = 3;
			break;
		case 3:
			dsi_dev[sel]->dsi_cfg1.bits.dpi_src_format  = 2;
			dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_format  = 0;
			break;
		default:
			dsi_dev[sel]->dsi_cfg1.bits.dpi_src_format  = 0;
			dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_format  = 5;
			break;
		}

		dsi_dev[sel]->dsi_dpi_cfg0.bits.dpi_vid           		=	vc;
		dsi_dev[sel]->dsi_dpi_cfg1.bits.de_ploarity  			=	0;
		dsi_dev[sel]->dsi_dpi_cfg1.bits.vsync_ploarity   		=	1;
		dsi_dev[sel]->dsi_dpi_cfg1.bits.hsync_ploarity   		=	1;
		dsi_dev[sel]->dsi_dpi_cfg1.bits.shutd_polarity   		=	0;
		dsi_dev[sel]->dsi_dpi_cfg1.bits.colorm_polarity  		=	0;
		dsi_dev[sel]->dsi_dpi_cfg0.bits.video_mode_format_18bit =	(format==1)?1:0;
		if(panel->lcd_dsi_if == LCD_DSI_IF_VIDEO_MODE)
		{
			dsi_dev[sel]->dsi_vid_ctl0.bits.video_mode_cfg     	= 0;
			dsi_dev[sel]->dsi_vid_ctl0.bits.hfp_lp_en          	= 0;//burst
		}
		else
		{
			dsi_dev[sel]->dsi_vid_ctl0.bits.video_mode_cfg     	= 2;
			dsi_dev[sel]->dsi_vid_ctl0.bits.hfp_lp_en          	= 1;
		}
		dsi_dev[sel]->dsi_vid_ctl0.bits.vsa_lp_en          	= 1;
		dsi_dev[sel]->dsi_vid_ctl0.bits.vbp_lp_en          	= 1;
		dsi_dev[sel]->dsi_vid_ctl0.bits.vfp_lp_en        	= 1;
		dsi_dev[sel]->dsi_vid_ctl0.bits.vact_lp_en         	= 1;
		dsi_dev[sel]->dsi_vid_ctl0.bits.hbp_lp_en          	= 0;

		dsi_dev[sel]->dsi_vid_ctl1.bits.pkt_multi_en       	= 0;
		dsi_dev[sel]->dsi_vid_ctl1.bits.pkt_null_in_hact   	= 0;
		dsi_dev[sel]->dsi_vid_ctl1.bits.bta_per_frame      	= 0;
		dsi_dev[sel]->dsi_vid_ctl1.bits.lp_cmd_en          	= 0;

		dsi_dev[sel]->dsi_vid_tim2.bits.pixels_per_pkg 		= x;
		dsi_dev[sel]->dsi_vid_ctl1.bits.pkt_num_per_line	= 1;
		dsi_dev[sel]->dsi_vid_ctl1.bits.pkt_null_size      	= 0;

		dsi_dev[sel]->dsi_vid_tim3.bits.hsa		       		= hspw      *dsi_bits_per_pixel[format]/(8*lane);// - (4+4+2);
		dsi_dev[sel]->dsi_vid_tim3.bits.hbp		       		= (hbp-hspw)*dsi_bits_per_pixel[format]/(8*lane);// - (4+2);
		dsi_dev[sel]->dsi_vid_tim2.bits.ht		     		= ht        *dsi_bits_per_pixel[format]/(8*lane);

		dsi_dev[sel]->dsi_vid_tim0.bits.vsa       			= vspw;
		dsi_dev[sel]->dsi_vid_tim0.bits.vbp		       		= vbp-vspw;
		dsi_dev[sel]->dsi_vid_tim1.bits.vfp		       		= vt-(vbp+y);
		dsi_dev[sel]->dsi_vid_tim1.bits.vact		  		= y;

		//dsi_dev[sel]->dsi_vid_ctl0.bits.en_video_mode	  = 1;

		dsi_dev[sel]->dsi_cfg1.bits.dpi_shut_down  = 0;
		dsi_dev[sel]->dsi_cfg1.bits.dpi_color_mode = 0;
		//dsi_dev[sel]->dsi_cfg1.bits.dpi_src = 1;

	}
	return 0;
}


__s32 dsi_cfg(__u32 sel,disp_panel_para * panel)
{
	dsi_irq_disable_all(sel);
  	dsi_basic_cfg(sel,panel);
	dsi_packet_cfg(sel,panel);

	return 0;
}


__s32 dsi_exit(__u32 sel)
{
	return 0;
}

__u8 dsi_ecc_pro(__u32 dsi_ph)
{
	return 0;
}

__u16 dsi_crc_pro(__u8* pd_p,__u32 pd_bytes)
{
	return 0;
}

__u16 dsi_crc_pro_pd_repeat(__u8 pd,__u32 pd_bytes)
{
	return 0;
}

#ifdef __LINUX_OSAL__
EXPORT_SYMBOL(dsi_dcs_wr);
EXPORT_SYMBOL(dsi_dcs_wr_0para);
EXPORT_SYMBOL(dsi_dcs_wr_1para);
EXPORT_SYMBOL(dsi_dcs_wr_2para);
EXPORT_SYMBOL(dsi_dcs_wr_3para);
EXPORT_SYMBOL(dsi_dcs_wr_4para);
EXPORT_SYMBOL(dsi_dcs_wr_5para);
#endif
