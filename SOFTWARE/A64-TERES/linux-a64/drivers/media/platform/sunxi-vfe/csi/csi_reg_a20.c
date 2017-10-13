/*
 * sunxi csi register read/write interface
*/

#include "csi_reg_a20.h"
#include "../utility/vfe_io.h"

#define ADDR_BIT_R_SHIFT 2
#define CLK_POL 1

volatile void __iomem *csi_base_addr[2];
enum csi_input_fmt input_fmt;

int csi_set_base_addr(unsigned int sel, unsigned long addr)
{
	if(sel > MAX_CSI-1)
		return -1;
  	csi_base_addr[sel] = (volatile void __iomem *)addr;

	return 0;
}

/* open module */
void csi_enable(unsigned int sel)
{
	vfe_reg_writel(csi_base_addr[sel] + CSI_REG_EN, 0x1);
}

void csi_disable(unsigned int sel)
{
	vfe_reg_clr(csi_base_addr[sel] + CSI_REG_EN, 0X1 << 0);
}

/* configure */
void csi_if_cfg(unsigned int sel, struct csi_if_cfg *csi_if_cfg)
{

}

void csi_timing_cfg(unsigned int sel, struct csi_timing_cfg *csi_tmg_cfg)
{
	vfe_reg_set(csi_base_addr[sel] + CSI_REG_CONF, csi_tmg_cfg->vref << 2 | /* [2] */
												csi_tmg_cfg->href << 1 |/* [1] */
												csi_tmg_cfg->sample);  /* [0] */
}

void csi_fmt_cfg(unsigned int sel, unsigned int ch, struct csi_fmt_cfg *csi_fmt_cfg)
{
	vfe_reg_set(csi_base_addr[sel] + CSI_REG_CONF, csi_fmt_cfg->input_fmt << 20 | /* [21:20] */
							  csi_fmt_cfg->output_fmt<< 16 | /* [18:16] */
							  csi_fmt_cfg->field_sel << 10 | /* [11:10] */
							  csi_fmt_cfg->input_seq<< 8); /* [9:8] */
	input_fmt = csi_fmt_cfg->input_fmt;
}


/* buffer */
void csi_set_buffer_address(unsigned int sel, unsigned int ch, enum csi_buf_sel buf, u64 addr)
{
	//bufer0a +4 = buffer0b, bufer0a +8 = buffer1a
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_BUF_0_A + (buf<<2), addr); 
}

u64 csi_get_buffer_address(unsigned int sel, unsigned int ch, enum csi_buf_sel buf)
{
	u32 t;
	t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_BUF_0_A + (buf<<2));
	return t;
}

/* capture */
void csi_capture_start(unsigned int sel, unsigned int ch_total_num, enum csi_cap_mode csi_cap_mode)
{
	if(CSI_VCAP == csi_cap_mode)
		vfe_reg_set(csi_base_addr[sel] + CSI_REG_CTRL, 0X1<<1);
	else
		vfe_reg_set(csi_base_addr[sel] + CSI_REG_CTRL, 0X1<<0);
}

void csi_capture_stop(unsigned int sel, unsigned int ch_total_num, enum csi_cap_mode csi_cap_mode)
{
    vfe_reg_clr(csi_base_addr[sel] + CSI_REG_CTRL, 0X3);
}

void csi_capture_get_status(unsigned int sel, unsigned int ch, struct csi_capture_status *status)
{
    u32 t;
    t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_STATUS);
    status->picture_in_progress = t&0x1;
    status->video_in_progress = (t>>1)&0x1;
}

/* size */
void csi_set_size(unsigned int sel, unsigned int ch, unsigned int length_h, unsigned int length_v, unsigned int buf_length_h, unsigned int buf_length_c)
{
	u32 t;
	
	switch(input_fmt){
	case CSI_CCIR656://TODO
	case CSI_YUV422:
		length_h = length_h*2;
		break;
	default:
		break;
	}

	t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_RESIZE_H);
	t = (t&0x0000ffff)|(length_h<<16);
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_RESIZE_H, t);
    
    t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_RESIZE_V);
    t = (t&0x0000ffff)|(length_v<<16);
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_RESIZE_V, t);
    
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_BUF_LENGTH, buf_length_h);

}

/* offset */
void csi_set_offset(unsigned int sel, unsigned int ch, unsigned int start_h, unsigned int start_v)
{
    u32 t;
    
    t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_RESIZE_H);
    t = (t&0xffff0000)|start_h;
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_RESIZE_H, t);
    
    t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_RESIZE_V);
    t = (t&0xffff0000)|start_v;
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_RESIZE_V, t);
}

/* interrupt */
void csi_int_enable(unsigned int sel, unsigned int ch, enum csi_int_sel interrupt)
{
    vfe_reg_set(csi_base_addr[sel] + CSI_REG_INT_EN, interrupt);
}

void csi_int_disable(unsigned int sel, unsigned int ch, enum csi_int_sel interrupt)
{
    vfe_reg_clr(csi_base_addr[sel] + CSI_REG_INT_EN, interrupt);
}

void inline csi_int_get_status(unsigned int sel, unsigned int ch,struct csi_int_status *status)
{
    u32 t;
    t = vfe_reg_readl(csi_base_addr[sel] + CSI_REG_INT_STATUS);

    status->capture_done     = t&CSI_INT_CAPTURE_DONE;
    status->frame_done       = t&CSI_INT_FRAME_DONE;
    status->buf_0_overflow   = t&CSI_INT_BUF_0_OVERFLOW;
    status->buf_1_overflow   = t&CSI_INT_BUF_1_OVERFLOW;
    status->buf_2_overflow   = t&CSI_INT_BUF_2_OVERFLOW;
    status->protection_error  = t&CSI_INT_PROTECTION_ERROR;
    status->hblank_overflow  = t&CSI_INT_HBLANK_OVERFLOW;
    status->vsync_trig		 = t&CSI_INT_VSYNC_TRIG;
}

void inline csi_int_clear_status(unsigned int sel, unsigned int ch, enum csi_int_sel interrupt)
{
    vfe_reg_writel(csi_base_addr[sel] + CSI_REG_INT_STATUS, interrupt);
}

