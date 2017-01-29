#include<clk/clk_plat.h>
#include<div64.h>
#include"clk_periph.h"
//#include"clk.h"
static u8 sunxi_clk_periph_get_parent(struct clk_hw *hw)
{
    u8 parent;
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);

    if(!periph->mux.reg)
        return 0;

    reg = periph_readl(periph,periph->mux.reg);
    parent = GET_BITS(periph->mux.shift, periph->mux.width, reg); 
    
    return parent;
}


static int sunxi_clk_periph_set_parent(struct clk_hw *hw, u8 index)
{
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);

    if(periph->flags & CLK_READONLY)
        return 0;

    if(!periph->mux.reg)
        return 0;

    reg = periph_readl(periph,periph->mux.reg);
    reg = SET_BITS(periph->mux.shift, periph->mux.width, reg, index);
    periph_writel(periph,reg, periph->mux.reg);

    return 0;
}

static int __sunxi_clk_periph_enable_shared(struct sunxi_clk_periph *periph)
{
    unsigned long reg;
    struct sunxi_clk_periph_gate *gate = &periph->gate;

    if(!periph->com_gate)
        return -1;        

    if(!periph->com_gate->val)
    {
        /* de-assert module */
        if(gate->reset && !(periph->flags & CLK_IGNORE_AUTORESET) && IS_SHARE_RST_GATE(periph)) {
            reg = periph_readl(periph,gate->reset);
            reg = SET_BITS(gate->rst_shift, 1, reg, 1);
            periph_writel(periph,reg, gate->reset);
        }
        /* enable bus gating */
        if(gate->bus && IS_SHARE_BUS_GATE(periph)) {
            reg = periph_readl(periph,gate->bus);
            reg = SET_BITS(gate->bus_shift, 1, reg, 1);
            periph_writel(periph,reg, gate->bus);
        }

        /* enable module gating */
        if(gate->enable&& IS_SHARE_MOD_GATE(periph)) {
            reg = periph_readl(periph,gate->enable);
            reg = SET_BITS(gate->enb_shift, 1, reg, 1);
            periph_writel(periph,reg, gate->enable);
        }

        /* enable dram gating */
        if(gate->dram&& IS_SHARE_MBUS_GATE(periph)) {
            reg = periph_readl(periph,gate->dram);
            reg = SET_BITS(gate->ddr_shift, 1, reg, 1);
            periph_writel(periph,reg, gate->dram);
        }       
    }
    periph->com_gate->val |= 1 << periph->com_gate_off;

    return 0;
}
static int sunxi_clk_periph_enable_shared(struct sunxi_clk_periph *periph)
{
    int ret = 0;
    if(!periph->com_gate)
        return -1;        

    ret = __sunxi_clk_periph_enable_shared(periph);
    return ret;        
}
static int __sunxi_clk_periph_enable(struct clk_hw *hw)
{
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    struct sunxi_clk_periph_gate *gate = &periph->gate;

    /* de-assert module */
    if(gate->reset && !(periph->flags & CLK_IGNORE_AUTORESET) && !IS_SHARE_RST_GATE(periph)) {
        reg = periph_readl(periph,gate->reset);
        reg = SET_BITS(gate->rst_shift, 1, reg, 1);
        periph_writel(periph,reg, gate->reset);
    }

    /* enable bus gating */
    if(gate->bus && !IS_SHARE_BUS_GATE(periph)) {
        reg = periph_readl(periph,gate->bus);
        reg = SET_BITS(gate->bus_shift, 1, reg, 1);
        periph_writel(periph,reg, gate->bus);
    }

    /* enable module gating */
    if(gate->enable&& !IS_SHARE_MOD_GATE(periph)) {
        reg = periph_readl(periph,gate->enable);
        if(periph->flags & CLK_REVERT_ENABLE)
            reg = SET_BITS(gate->enb_shift, 1, reg, 0);
        else
            reg = SET_BITS(gate->enb_shift, 1, reg, 1);
        periph_writel(periph,reg, gate->enable);
    }

    /* enable dram gating */
    if(gate->dram&& !IS_SHARE_MBUS_GATE(periph)) {
        reg = periph_readl(periph,gate->dram);
        reg = SET_BITS(gate->ddr_shift, 1, reg, 1);
        periph_writel(periph,reg, gate->dram);

    }

    return 0;
}
static int sunxi_clk_periph_enable(struct clk_hw *hw)
{
    int ret = 0;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    if(periph->flags & CLK_READONLY)
        return 0;

    if(periph->com_gate)
        sunxi_clk_periph_enable_shared(periph);

    ret = __sunxi_clk_periph_enable(hw);

    return ret;
}
static int __sunxi_clk_periph_is_enabled(struct clk_hw *hw)
{
    int state = 1;
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    struct sunxi_clk_periph_gate *gate = &periph->gate;

    /* enable bus gating */
    if(gate->bus) {
        reg = periph_readl(periph,gate->bus);
        state &= GET_BITS(gate->bus_shift, 1, reg);
    }

    /* enable module gating */
    if(gate->enable) {
        reg = periph_readl(periph,gate->enable);
        state &= GET_BITS(gate->enb_shift, 1, reg);
    }
    /* de-assert module */
    if(gate->reset) {
        reg = periph_readl(periph,gate->reset);
        state &= GET_BITS(gate->rst_shift, 1, reg);
    }

    /* enable dram gating */
    if(gate->dram) {
        reg = periph_readl(periph,gate->dram);
        state &= GET_BITS(gate->ddr_shift, 1, reg);
    }

    return state;
}
static int sunxi_clk_periph_is_enabled(struct clk_hw *hw)
{
    int state = 0;
    //struct sunxi_clk_periph *periph = to_clk_periph(hw);

    state = __sunxi_clk_periph_is_enabled(hw);    
    return state;
}
static void __sunxi_clk_periph_disable_shared(struct sunxi_clk_periph *periph)
{
    unsigned long reg;
    struct sunxi_clk_periph_gate *gate = &periph->gate;
    if(!periph->com_gate->val)
     return ;

    periph->com_gate->val &= ~(1 << periph->com_gate_off);      

    if(!periph->com_gate->val)
    {
        /* disable dram gating */
        if(gate->dram&& IS_SHARE_MBUS_GATE(periph)) {
            reg = periph_readl(periph,gate->dram);
            reg = SET_BITS(gate->ddr_shift, 1, reg, 0);
            periph_writel(periph,reg, gate->dram);
        }

        /* disable module gating */
        if(gate->enable&& IS_SHARE_MOD_GATE(periph)) {
            reg = periph_readl(periph,gate->enable);
            reg = SET_BITS(gate->enb_shift, 1, reg, 0);
            periph_writel(periph,reg, gate->enable);
        }

        /* disable bus gating */
        if(gate->bus&& IS_SHARE_BUS_GATE(periph)) {
            reg = periph_readl(periph,gate->bus);
            reg = SET_BITS(gate->bus_shift, 1, reg, 0);
            periph_writel(periph,reg, gate->bus);
        }
        /* assert module */
        if(gate->reset && !(periph->flags & CLK_IGNORE_AUTORESET) && IS_SHARE_RST_GATE(periph)) {
            reg = periph_readl(periph,gate->reset);
            reg = SET_BITS(gate->rst_shift, 1, reg, 0);
            periph_writel(periph,reg, gate->reset);
        }
    }

}
static void sunxi_clk_periph_disable_shared(struct sunxi_clk_periph *periph)
{
    if(!periph->com_gate->val)
     return ;

	__sunxi_clk_periph_disable_shared(periph);         
}
static void __sunxi_clk_periph_disable(struct sunxi_clk_periph *pperiph)
{
    unsigned long reg;
    struct sunxi_clk_periph *periph = pperiph;
    struct sunxi_clk_periph_gate *gate = &periph->gate;

    /* disable dram gating */
    if(gate->dram&& !IS_SHARE_MBUS_GATE(periph)) {
        reg = periph_readl(periph,gate->dram);
        reg = SET_BITS(gate->ddr_shift, 1, reg, 0);
        periph_writel(periph,reg, gate->dram);
    }

    /* disable module gating */
    if(gate->enable&& !IS_SHARE_MOD_GATE(periph)) {
        reg = periph_readl(periph,gate->enable);
        if(periph->flags & CLK_REVERT_ENABLE)
            reg = SET_BITS(gate->enb_shift, 1, reg, 1);
        else
            reg = SET_BITS(gate->enb_shift, 1, reg, 0);

        periph_writel(periph,reg, gate->enable);
    }

    /* disable bus gating */
    if(gate->bus&& !IS_SHARE_BUS_GATE(periph)) {
        reg = periph_readl(periph,gate->bus);
        reg = SET_BITS(gate->bus_shift, 1, reg, 0);
        periph_writel(periph,reg, gate->bus);
    }

    /* assert module */
    if(gate->reset && !(periph->flags & CLK_IGNORE_AUTORESET) &&!IS_SHARE_RST_GATE(periph)) {
        reg = periph_readl(periph,gate->reset);
        reg = SET_BITS(gate->rst_shift, 1, reg, 0);
        periph_writel(periph,reg, gate->reset);
    }
}
static void sunxi_clk_periph_disable(struct clk_hw *hw)
{
    struct sunxi_clk_periph *periph = to_clk_periph(hw);

    if(periph->flags & CLK_READONLY)
        return ;

    __sunxi_clk_periph_disable(periph);    
    if(periph->com_gate)
        sunxi_clk_periph_disable_shared(periph);        
}

static unsigned long sunxi_clk_periph_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    struct sunxi_clk_periph_div *divider = &periph->divider;
    unsigned long div, div_m = 0, div_n = 0;
    unsigned long long rate = parent_rate;

    if(!divider->reg)
        return parent_rate;

    reg = periph_readl(periph,divider->reg);
    if(divider->mwidth)
        div_m = GET_BITS(divider->mshift, divider->mwidth, reg);
    if(divider->nwidth)
        div_n = GET_BITS(divider->nshift, divider->nwidth, reg);
    div = (div_m+1)*(1<<div_n);
    do_div(rate, div);

    return rate;
}


static long sunxi_clk_periph_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *prate)
{
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    struct sunxi_clk_periph_div *divider = &periph->divider;
    unsigned long i=0,factor_m=0,factor_n=0,found=0;
    unsigned long div, div_m = 0, div_n = 0;
    unsigned long long parent_rate = (*prate+rate/2-1);

    do_div(parent_rate, rate);	
	div = parent_rate;
    if(!div)
        return *prate;

    parent_rate = *prate;
    div_m = 1<<divider->mwidth;
    if(divider->nwidth) {
        div_n = 1<<divider->nwidth;
        div_n = 1<<(div_n-1);
    } else
        div_n = 1;

    while(i < (1<<divider->nwidth))
    {
        if(div <= div_m)
        {
            factor_m = div-1;
            factor_n = i;
            do_div(parent_rate, (factor_m+1)*(1 << factor_n));
            found = 1;
            break;
        }
        div = div >>1;
        i++;

         if(!div)
        {
            factor_m = 0;
            factor_n = i;
            do_div(parent_rate, (factor_m+1)*(1 << factor_n));
            found = 1;
            break;
        }
    }
    if(!found)
    {
        factor_m = (div >div_m?div_m:div)-1;
        factor_n = (1<<divider->nwidth) -1;
        do_div(parent_rate, (factor_m+1)*(1 << factor_n));
    }

    return parent_rate;
}

static int __sunxi_clk_periph_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    unsigned long i=0,factor_m=0,factor_n=0,found=0;
    unsigned long reg;
    struct sunxi_clk_periph *periph = to_clk_periph(hw);
    struct sunxi_clk_periph_div *divider = &periph->divider;
    unsigned long div, div_m = 0, div_n = 0;
    unsigned long long tmp_rate = parent_rate;

	if(periph->flags & CLK_READONLY)
        return 0;

	if(!divider->reg)
		return 0;

    do_div(tmp_rate, rate);
    div = tmp_rate;
    if(!div)
        div_m = div_n =0;
    else {
        div_m = 1<<divider->mwidth;
        div_n = (1<<divider->nwidth)-1;
		
		if( div > (div_m<<div_n) )
		{
			//WARN(1, "clk %s rate is too large : %lu\n",hw->clk->name , rate );
			div = div_m<<div_n;
		}
    found = 0;
    while(i < (1<<divider->nwidth))
    {
        if(div <= div_m)
        {
            factor_m = div-1;
            factor_n = i;
            found = 1;
            break;
        }
        div = div >>1;
        i++;
         if(!div)
        {
            factor_m = 0;
            factor_n = i;
            found = 1;
            break;
        }
    }
    if(!found)
    {
        factor_m = (div >div_m?div_m:div)-1;
        factor_n = (1<<divider->nwidth) -1;
    }
    div_m = factor_m;
    div_n = factor_n;
    }

    reg = periph_readl(periph,divider->reg);
    if(divider->mwidth)
        reg = SET_BITS(divider->mshift, divider->mwidth, reg, div_m);
    if(divider->nwidth)
        reg = SET_BITS(divider->nshift, divider->nwidth, reg, div_n);
    periph_writel(periph,reg, divider->reg);

    return 0;
}
static int sunxi_clk_periph_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;
    //struct sunxi_clk_periph *periph = to_clk_periph(hw);

    ret = __sunxi_clk_periph_set_rate(hw,rate,parent_rate);
    return ret;
}

static const struct clk_ops sunxi_clk_periph_ops = {

    .get_parent = sunxi_clk_periph_get_parent,
    .set_parent = sunxi_clk_periph_set_parent,

    .recalc_rate = sunxi_clk_periph_recalc_rate,
    .round_rate = sunxi_clk_periph_round_rate,
    .set_rate = sunxi_clk_periph_set_rate,

    .is_enabled = sunxi_clk_periph_is_enabled,
    .enable = sunxi_clk_periph_enable,
    .disable = sunxi_clk_periph_disable,
};
void sunxi_clk_get_periph_ops(struct clk_ops* ops)
{
    memcpy(ops,&sunxi_clk_periph_ops,sizeof(sunxi_clk_periph_ops));
}

int sunxi_clk_register_periph(const char *name,
            const char **parent_names, int num_parents,unsigned long flags,
            void  *base, struct sunxi_clk_periph *periph)
{
    struct clk_init_data init;
	struct clk *clk;
#ifdef __SUNXI_ALL_CLK_IGNORE_UNUSED__
		flags |= CLK_IGNORE_UNUSED;
#endif
    init.name = name;
    init.ops = periph->priv_clkops?periph->priv_clkops:&sunxi_clk_periph_ops;
    init.flags = flags;
    init.parent_names = parent_names;
    init.num_parents = num_parents;

    /* Data in .init is copied by clk_register(), so stack variable OK */
    periph->hw.init = &init;
    periph->flags = init.flags;
    /* fix registers */
    periph->mux.reg = periph->mux.reg ? (base + (u32)periph->mux.reg) : NULL;
    periph->divider.reg = periph->divider.reg ? (base + (u32)periph->divider.reg) : NULL;

	//printf("%s: periph_mux_reg_address = 0x%x, periph_mux_reg_address = 0x%x\n",__func__, (u32)periph->mux.reg, (u32)periph->divider.reg);
    periph->gate.enable = periph->gate.enable ? (base + (u32)periph->gate.enable) : NULL;
    periph->gate.reset = periph->gate.reset ? (base + (u32)periph->gate.reset) : NULL;
    periph->gate.bus = periph->gate.bus ? (base + (u32)periph->gate.bus) : NULL;
    periph->gate.dram = periph->gate.dram ? (base + (u32)periph->gate.dram) : NULL;

	clk = clk_register(&periph->hw);
	if (NULL == clk)
		return -1;
    return 0;
}

