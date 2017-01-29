#include "de_tvec_i.h"
#include "drv_tv_i.h"

u32 tve_reg_base0;
u32 tve_reg_base1;

void tv_low_print_base_reg(void)
{
	/*u32 i = 0;
	u32 * preg = NULL;
	pr_debug("<4>[TV] enter tv_low_print_base_reg");
	pr_debug("<4>tve_reg_base0 = %u", tve_reg_base0);
	pr_debug("<4>tve_reg_base1 = %u", tve_reg_base1);
	preg = (u32 *)tve_reg_base0;
	for(i = 0; i < 125; i++) {
		pr_debug("<4>[TV]vitule tve_reg_base0 reg%d %u\n",i ,*(preg + i));
	}
	pr_debug("<4>[TV] *****************************");
	preg = (u32 *)tve_reg_base1;
	for(i = 0; i < 125; i++) {
		pr_debug("<4>[TV]vitule tve_reg_base1 reg%d %u\n",i ,*(preg + i));
	}
	*/
}

s32 tve_low_set_reg_base(u32 sel,u32 address)
{
	if(sel == 0) {
		tve_reg_base0 = g_tv_info.screen[sel].base_address;
		tve_reg_base1 = tve_reg_base0;									//sel 1 from disp_driver map to sel0 tv regbase
	}
	else if(sel == 1) {
		tve_reg_base1 = g_tv_info.screen[sel].base_address;
	}
	return 0;
}

//tve
// init module
////////////////////////////////////////////////////////////////////////////////
s32  tve_low_init(u32 sel,u32 cali)
{
	TVE_WUINT32(sel, TVE_304,cali<<16 | 0xc<<8);
	TVE_WUINT32(sel, TVE_008,0x433e12b0);
	return 0;
}

s32 tve_low_exit(u32 sel)
{
	return 0;
}

// open module
////////////////////////////////////////////////////////////////////////////////

s32 tve_low_open(u32 sel)
{
	TVE_CLR_BIT(sel,TVE_000,0x1<<31);
	TVE_SET_BIT(sel,TVE_000,0x1<<0);
	TVE_SET_BIT(sel,TVE_008,0x1<<0);
	return 0;
}

s32 tve_low_close(u32 sel)
{
	TVE_CLR_BIT(sel,TVE_008,0x1<<0);
	TVE_CLR_BIT(sel,TVE_000,0x1<<0);
	TVE_SET_BIT(sel,TVE_000,0x1<<31);
      return 0;
}

// set mode
////////////////////////////////////////////////////////////////////////////////
//15~13     12~10       9~7         6~4
//DAC3      DAC2        DAC1        DAC0

//CVBS(0)
//             CR(4)         CB(5)        Y(6)
//                            Chroma(1)    Luma(2)


void tve_low_dac_cfg(u32 sel, u8 mode)
{
}

__s32 TVE_resync_enable(__u32 sel)
{
	TVE_SET_BIT(sel,TVE_130,0x1<<30);
    return 0;
}

__s32 TVE_resync_disable(__u32 sel)
{
	TVE_CLR_BIT(sel,TVE_130,0x1<<30);
    return 0;
}


s32 tve_low_set_tv_mode(u32 sel, u8 mode)
{
	if(14==mode)
	{
		TVE_WUINT32(sel,TVE_000,0x00000300);
		TVE_WUINT32(sel,TVE_004,0x07070000);
		TVE_WUINT32(sel,TVE_00C,0x30001400);
		TVE_WUINT32(sel,TVE_010,0x21F07C1F);
		TVE_WUINT32(sel,TVE_014,0x00760020);
		TVE_WUINT32(sel,TVE_018,0x00000016);
		TVE_WUINT32(sel,TVE_01C,0x0016020D);
		TVE_WUINT32(sel,TVE_020,0x00F0011A);
		TVE_WUINT32(sel,TVE_100,0x00000001);
		TVE_WUINT32(sel,TVE_104,0x00000000);
		TVE_WUINT32(sel,TVE_108,0x00000002);
		TVE_WUINT32(sel,TVE_10C,0x0000004F);
		TVE_WUINT32(sel,TVE_110,0x00000000);
		TVE_WUINT32(sel,TVE_114,0x0016447E);
		TVE_WUINT32(sel,TVE_118,0x0000A0A0);
		TVE_WUINT32(sel,TVE_11C,0x001000F0);
		TVE_WUINT32(sel,TVE_120,0x01E80320);
		TVE_WUINT32(sel,TVE_124,0x000005A0);
		TVE_WUINT32(sel,TVE_128,0x00010000);
		TVE_WUINT32(sel,TVE_12C,0x00000101);
		TVE_WUINT32(sel,TVE_130,0x20050368);
		TVE_WUINT32(sel,TVE_134,0x00000000);
		TVE_WUINT32(sel,TVE_138,0x00000000);
		TVE_WUINT32(sel,TVE_13C,0x00000000);
	}
	else if(11==mode)
	{
		TVE_WUINT32(sel,TVE_000,0x00000300);
		TVE_WUINT32(sel,TVE_004,0x07070001);
		TVE_WUINT32(sel,TVE_00C,0x30001400);
		TVE_WUINT32(sel,TVE_010,0x2A098ACB);
		TVE_WUINT32(sel,TVE_014,0x008A0018);
		TVE_WUINT32(sel,TVE_018,0x00000016);
		TVE_WUINT32(sel,TVE_01C,0x00160271);
		TVE_WUINT32(sel,TVE_020,0x00FC00FC);
		TVE_WUINT32(sel,TVE_100,0x00000000);
		TVE_WUINT32(sel,TVE_104,0x00000001);
		TVE_WUINT32(sel,TVE_108,0x00000005);
		TVE_WUINT32(sel,TVE_10C,0x00002929);
		TVE_WUINT32(sel,TVE_110,0x00000000);
		TVE_WUINT32(sel,TVE_114,0x0016447E);
		TVE_WUINT32(sel,TVE_118,0x0000A8A8);
		TVE_WUINT32(sel,TVE_11C,0x001000FC);
		TVE_WUINT32(sel,TVE_120,0x01E80320);
		TVE_WUINT32(sel,TVE_124,0x000005A0);
		TVE_WUINT32(sel,TVE_128,0x00010000);
		TVE_WUINT32(sel,TVE_12C,0x00000101);
		TVE_WUINT32(sel,TVE_130,0x2005000A);
		TVE_WUINT32(sel,TVE_134,0x00000000);
		TVE_WUINT32(sel,TVE_138,0x00000000);
		TVE_WUINT32(sel,TVE_13C,0x00000000);
	}
	return 0;
}

__u32 TVE_filter_mode(__u32 sel,__u32 vector,__u32 lti)
{
	if(1==vector) {
		TVE_CLR_BIT(sel,TVE_000,0xf<<10);
		TVE_SET_BIT(sel,TVE_00C,0x1<<16);
	}else if (2==vector) {
		TVE_CLR_BIT(sel,TVE_00C,0x1<<16);
		TVE_SET_BIT(sel,TVE_000,0x5<<10);
	}else {
		TVE_CLR_BIT(sel,TVE_000,0xf<<10);
		TVE_CLR_BIT(sel,TVE_00C,0x1<<16);
	}
	if(0==lti)	{
		TVE_SET_BIT(sel,TVE_00C,0x1<<31);
	}else {
		TVE_CLR_BIT(sel,TVE_00C,0x1<<31);
		}
	return 0;
}

s32 tve_low_set_vga_mode(u32 sel)
{
	return 0;
}

u8 tve_low_query_int(u32 sel)
{
    u8    sts = 0;
    u32   readval;

    readval = TVE_RUINT32(sel,TVE_034);
    sts = readval & 0x0f;

    return sts;
}

u8  tve_low_clear_int(u32 sel)
{
    u32    sts = 0;
    u32    readval;

    readval = TVE_RUINT32(sel,TVE_034);
    sts = readval & 0x0f;
    TVE_WUINT32(sel,TVE_034,sts);

    return 0;
}

//0:unconnected; 1:connected; 3:short to ground
s32 tve_low_get_dac_status(u32 sel)  //index modify to sel
{
	__u32   readval;
	readval = TVE_RUINT32(sel,TVE_038);
	return (readval & 0x3);
}

u8 tve_low_dac_int_enable(u32 sel,u8 index)
{
	return 0;
}

u8 tve_low_dac_int_disable(u32 sel,u8 index)
{
	return 0;
}

u8 tve_low_dac_autocheck_enable(u32 sel, u8 index)
{
	TVE_WUINT32(sel, TVE_0F8,0x00000230);
	TVE_WUINT32(sel, TVE_0FC,0x028F00FF);		//20ms x 10
	TVE_WUINT32(sel, TVE_03C,0x00000004);		//1.0v refer for 0.71v/1.43v detect
	TVE_WUINT32(sel, TVE_030,0x00000001);		//detect enable
	return 0;
}

u8 tve_low_dac_autocheck_disable(u32 sel,u8 index)
{
	TVE_WUINT32(sel,TVE_030,0x0);
	TVE_WUINT32(sel,TVE_0F8,0x0);
	return 0;
}

u8 tve_low_dac_enable(u32 sel,u8 index)
{
	return 0;
}

u8 tve_low_dac_disable(u32 sel,u8 index)
{
	return 0;
}

s32 tve_low_dac_set_source(u32 sel,u32 index,u32 source)
{
	return 0;
}

s32 tve_low_dac_get_source(u32 sel,u32 index)
{
	return 0;
}

u8 tve_low_dac_set_de_bounce(__u32 sel,__u8 index,__u32 times)
{
	return 0;
}

u8 tve_low_dac_get_de_bounce(u32 sel,u8 index)
{
	return 0;
}

//dac: 0~3
//index: 0~3
s32 tve_low_dac_sel(u32 sel,u32 dac, u32 index)
{
	return 0;
}

u8 tve_low_csc_init(u32 sel,u8 type)
{
	return 0;
}

u8 tve_low_csc_enable(u32 sel)
{
	return 0;
}
u8 tve_low_csc_disable(u32 sel)
{

	return 0;
}


