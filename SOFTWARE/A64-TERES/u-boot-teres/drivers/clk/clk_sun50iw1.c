#include"clk_sun50iw1.h"
#include<clk/clk_plat.h>
#include"clk_factor.h"
#include"clk_periph.h"
#include"clk_sun50iw1_tbl.c"
#include<div64.h>
#define LOCKBIT(x) x
//#define do_div(x,y) ((x)=(x)/(y))

/*
ns	nw	ks	kw	ms	mw	ps	pw	d1s  d1w  d2s  d2w	{frac  out	mode}  en-s   sdmss  sdmsw	sdmpat		sdmval*/
//SUNXI_CLK_FACTORS(			pll_cpu,    8,  5,  4,  2,  0,  2,  16, 2,  0,   0,   0,   0,    0,    0,   0,     31,   24,     0,       PLL_CPUPAT,  0xd1303333);
//SUNXI_CLK_FACTORS(			pll_audio,  8,  7,  0,  0,  0,  5,  16, 4,  0,   0,   0,   0,    0,    0,   0,     31,    0,     0,       0,        	0);
SUNXI_CLK_FACTORS(			pll_video0, 8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_VIDEO0PAT,0xd1303333);
//SUNXI_CLK_FACTORS(			pll_ve,     8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_VEPAT,   0xd1303333);
//SUNXI_CLK_FACTORS_UPDATE(	pll_ddr0,   8,  5,  4,  2,  0,  2,  0,  0,  0,   0,   0,   0,    0,    0,   0, 	   31,	 24,     0,		  PLL_DRR0PAT, 0xd1303333 , 20);
//SUNXI_CLK_FACTORS(			pll_periph0,8,  5,  4,  2,  0,  0,  0,  0,  0,   0,   0,   0,    0,    0,   0,     31,    0,     0,       0,        	0);
//SUNXI_CLK_FACTORS(			pll_periph1,8,  5,  4,  2,  0,  0,  0,  0,  0,   0,   0,   0,    0,    0,   0,     31,   20,     0,       PLL_PERI1PAT,0xd1303333);
SUNXI_CLK_FACTORS(			pll_video1, 8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_VEDEO1PAT,0xd1303333);
//SUNXI_CLK_FACTORS(			pll_gpu, 	8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_GPUPAT,0xd1303333);
SUNXI_CLK_FACTORS(			pll_mipi,   8,  4,  4,  2,  0,  4,  0,  0,  0,   0,   0,   0,    0,    0,   0,     31,   20,     0,       PLL_MIPIPAT, 0xd1303333);
//SUNXI_CLK_FACTORS(			pll_hsic,   8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_HSICPAT ,0xd1303333);
SUNXI_CLK_FACTORS(			pll_de,     8,  7,  0,  0,  0,  4,  0,  0,  0,   0,   0,   0,    1,    25,  24,    31,   20,     0,       PLL_DEPAT   ,0xd1303333);
//SUNXI_CLK_FACTORS_UPDATE(	pll_ddr1,   8,  7,  0,  0,  0,  2,  0,  0,  0,   0,   0,   0,    0,    0,   0, 	   31,	 24,     0,		  PLL_DDR1PAT, 0xf1303333 , 30);

static int get_factors_pll_video0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllvideo0_max ? pllvideo0_max : rate;
    do_div(tmp_rate, 1000000);

    index = tmp_rate;
 		if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_video0,factor, factor_pllvideo0_tbl,index,sizeof(factor_pllvideo0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}

static int get_factors_pll_video1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllvideo1_max ? pllvideo1_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
 	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_video1,factor, factor_pllvideo1_tbl,index,sizeof(factor_pllvideo1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 		return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}

static int get_factors_pll_mipi(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

    unsigned long long tmp_rate;
    u32 delta1,delta2,want_rate,new_rate,save_rate=0;
    int n,k,m;
    if(!factor)
        return -1;
    tmp_rate = rate>1440000000 ? 1440000000 : rate;
    do_div(tmp_rate, 1000000);
    want_rate = tmp_rate;
    for(m=1;			m <=16;	m++)
		for(k=2;			k <=4;	k++)
            for(n=1;			n <=16;	n++)
            {
                new_rate = (parent_rate/1000000)*k*n/m;
                delta1 = (new_rate > want_rate)?(new_rate - want_rate):(want_rate - new_rate);
                delta2 =  (save_rate > want_rate)?(save_rate - want_rate):(want_rate - save_rate);
                if(delta1 < delta2)
                {
                    factor->factorn = n-1;
                    factor->factork = k-1;
                    factor->factorm = m-1;
                    save_rate = new_rate;
                }
            }

    return 0;
}

static int get_factors_pll_de(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllde_max ? pllde_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
    if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_de,factor, factor_pllde_tbl,index,sizeof(factor_pllde_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}

/*	pll_video0:24*N/M	*/
static unsigned long calc_rate_media(u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate = (parent_rate?parent_rate:24000000);
    if(factor->frac_mode == 0)
    {
        if(factor->frac_freq == 1)
          return 297000000;
        else
          return 270000000;
    }
    else
    {
        tmp_rate = tmp_rate * (factor->factorn+1);
        do_div(tmp_rate, factor->factorm+1);
        return (unsigned long)tmp_rate;
    }
}

/*	pll_mipi: pll_video0*N*K/M	*/
static unsigned long calc_rate_pll_mipi(u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate = (parent_rate?parent_rate:24000000);
    tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
    do_div(tmp_rate, factor->factorm+1);
    return (unsigned long)tmp_rate;
}

#if 0
static int get_factors_pll_cpu(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

   	int index;
    unsigned long long tmp_rate;
    if(!factor)
        return -1;
    tmp_rate = rate>pllcpu_max ? pllcpu_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
 		if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_cpu,factor, factor_pllcpu_tbl,index,sizeof(factor_pllcpu_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    return 0;
}

static int get_factors_pll_audio(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    if(rate == 22579200) {
        factor->factorn = 78;
        factor->factorm = 20;
        factor->factorp = 3;
    } else if(rate == 24576000) {
        factor->factorn = 85;
        factor->factorm = 20;
        factor->factorp = 3;
    } else
        return -1;

    return 0;
}



static int get_factors_pll_ve(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllve_max ? pllve_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
    if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ve,factor, factor_pllve_tbl,index,sizeof(factor_pllve_tbl)/sizeof(struct sunxi_clk_factor_freq)))
        return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}

static int get_factors_pll_ddr0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	unsigned long long tmp_rate;
	if (!factor)
		return -1;
	tmp_rate = rate > pllddr0_max ? pllddr0_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ddr0, factor,
					 factor_pllddr0_tbl, index,
					 sizeof(factor_pllddr0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}

static int get_factors_pll_periph0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
	unsigned long long tmp_rate;
	if(!factor)
	    return -1;
	tmp_rate = rate>pllperiph0_max ? pllperiph0_max : rate;
    do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_periph0,factor, factor_pllperiph0_tbl,index,sizeof(factor_pllperiph0_tbl)/sizeof(struct sunxi_clk_factor_freq)))
	    return -1;
	return 0;
}

static int get_factors_pll_periph1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
    unsigned long long tmp_rate;
    if(!factor)
        return -1;
    tmp_rate = rate>pllperiph1_max ? pllperiph1_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
        if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_periph1,factor, factor_pllperiph1_tbl,index,sizeof(factor_pllperiph1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    return 0;
}




static int get_factors_pll_gpu(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllgpu_max ? pllgpu_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
 		if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_gpu,factor, factor_pllgpu_tbl,index,sizeof(factor_pllgpu_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}



static int get_factors_pll_hsic(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    unsigned long long tmp_rate;
    int index;
    if(!factor)
        return -1;

    tmp_rate = rate>pllhsic_max ? pllhsic_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;
    if(sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_hsic,factor, factor_pllhsic_tbl,index,sizeof(factor_pllhsic_tbl)/sizeof(struct sunxi_clk_factor_freq)))
 			return -1;
    if(rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    }
    else if(rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}



static int get_factors_pll_ddr1(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
	int index;
    unsigned long long tmp_rate;
	if (!factor)
		return -1;
	tmp_rate = rate > pllddr1_max ? pllddr1_max : rate;
	do_div(tmp_rate, 1000000);
	index = tmp_rate;
	if (sunxi_clk_get_common_factors_search(&sunxi_clk_factor_pll_ddr1, factor,
					 factor_pllddr1_tbl, index,
					 sizeof(factor_pllddr1_tbl)/sizeof(struct sunxi_clk_factor_freq)))
		return -1;

	return 0;
}


/*	pll_cpux: 24*N*K/(M*P)	*/
static unsigned long calc_rate_pll_cpu(u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate = (parent_rate?parent_rate:24000000);
    tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
    do_div(tmp_rate, (factor->factorm+1) * (1 << factor->factorp));
    return (unsigned long)tmp_rate;
}
/*	pll_audio:24*N/(M*P)	*/
static unsigned long calc_rate_pll_audio(u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate = (parent_rate?parent_rate:24000000);
    if((factor->factorn == 78) && (factor->factorm == 20) && (factor->factorp == 3))
        return 22579200;
    else if((factor->factorn == 85) && (factor->factorm == 20) && (factor->factorp == 3))
        return 24576000;
    else
    {
        tmp_rate = tmp_rate * (factor->factorn+1);
        do_div(tmp_rate, (factor->factorm+1) * (factor->factorp+1));
        return (unsigned long)tmp_rate;
    }
}

/*	pll_ddr0:24*N*K/M	*/
static unsigned long calc_rate_pll_ddr0(u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate = (parent_rate?parent_rate:24000000);
    tmp_rate = tmp_rate * (factor->factorn+1) * (factor->factork+1);
    do_div(tmp_rate, factor->factorm+1);
    return (unsigned long)tmp_rate;
}
/*	pll_ddr1: 24*N/M	*/
static unsigned long calc_rate_pll_ddr1(u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate = (parent_rate?parent_rate:24000000);
    tmp_rate = tmp_rate * (factor->factorn+1) ;
    do_div(tmp_rate, factor->factorm+1);
    return (unsigned long)tmp_rate;
}
/*	pll_periph0:24*N*K/2	*/
static unsigned long calc_rate_pll_periph(u32 parent_rate, struct clk_factors_value *factor)
{
    return (unsigned long)(parent_rate?(parent_rate/2):12000000) * (factor->factorn+1) * (factor->factork+1);
}
#endif

u8 get_parent_pll_mipi(struct clk_hw *hw)
{
    u8 parent;
    unsigned long reg;
	struct sunxi_clk_factors *factor = to_clk_factor(hw);

    if(!factor->reg)
        return 0;
    reg = readl(factor->reg);
    parent = GET_BITS(21, 1, reg);

    return parent;
}
int set_parent_pll_mipi(struct clk_hw *hw, u8 index)
{
    unsigned long reg;
	struct sunxi_clk_factors *factor = to_clk_factor(hw);

    if(!factor->reg)
        return 0;
    reg = readl(factor->reg);
    reg = SET_BITS(21, 1, reg, index);
    writel(reg, factor->reg);
    return 0;
}
static int clk_enable_pll_mipi(struct clk_hw *hw)
{
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    if(config->sdmwidth)
    {
        writel(config->sdmval, (void __iomem *)config->sdmpat);
        reg = SET_BITS(config->sdmshift, config->sdmwidth, reg, 1);
    }

    reg |= 0x3 << 22;
    writel(reg, factor->reg);
    udelay(100);

    reg = SET_BITS(config->enshift, 1, reg, 1);
    writel(reg, factor->reg);
    udelay(100);

    return 0;
}

static void clk_disable_pll_mipi(struct clk_hw *hw)
{
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    if(config->sdmwidth)
        reg = SET_BITS(config->sdmshift, config->sdmwidth, reg, 0);
    reg = SET_BITS(config->enshift, 1, reg, 0);
    reg &= ~(0x3 << 22);
    writel(reg, factor->reg);
}

static const char *mipi_parents[] = {"pll_video0",""};
static const char *hosc_parents[] = {"hosc"};
struct clk_ops pll_mipi_ops;

struct factor_init_data sunxi_factos[] = {
     /* name         parent        parent_num, flags       reg       	lock_reg     lock_bit    config                          get_factors          		calc_rate       priv_ops*/
    //{"pll_cpu",     hosc_parents, 1,CLK_GET_RATE_NOCACHE, PLL_CPU,    PLL_CPU,     LOCKBIT(28),&sunxi_clk_factor_pll_cpu,    	&get_factors_pll_cpu,    &calc_rate_pll_cpu     ,(struct clk_ops*)NULL},
    //{"pll_audio",   hosc_parents, 1,          0,          PLL_AUDIO,  PLL_AUDIO,   LOCKBIT(28),&sunxi_clk_factor_pll_audio,  	&get_factors_pll_audio,  &calc_rate_pll_audio   ,(struct clk_ops*)NULL},
    {"pll_video0",  hosc_parents, 1,          0,          PLL_VIDEO0, PLL_VIDEO0,  LOCKBIT(28),&sunxi_clk_factor_pll_video0,  	&get_factors_pll_video0, &calc_rate_media       ,(struct clk_ops*)NULL},
    //{"pll_ve",      hosc_parents, 1,          0,          PLL_VE,     PLL_VE,      LOCKBIT(28),&sunxi_clk_factor_pll_ve,     	&get_factors_pll_ve,     &calc_rate_media       ,(struct clk_ops*)NULL},
    //{"pll_ddr0",    hosc_parents, 1,CLK_GET_RATE_NOCACHE, PLL_DDR0,   PLL_DDR0,    LOCKBIT(28),&sunxi_clk_factor_pll_ddr0,   	&get_factors_pll_ddr0,   &calc_rate_pll_ddr0    ,(struct clk_ops*)NULL},
    //{"pll_periph0", hosc_parents, 1,          0,          PLL_PERIPH0,PLL_PERIPH0, LOCKBIT(28),&sunxi_clk_factor_pll_periph0,	&get_factors_pll_periph0,&calc_rate_pll_periph  ,(struct clk_ops*)NULL},
    //{"pll_periph1", hosc_parents, 1,          0,          PLL_PERIPH1,PLL_PERIPH1, LOCKBIT(28),&sunxi_clk_factor_pll_periph1,	&get_factors_pll_periph1,&calc_rate_pll_periph  ,(struct clk_ops*)NULL},
    {"pll_video1",  hosc_parents, 1,          0,          PLL_VIDEO1, PLL_VIDEO1,  LOCKBIT(28),&sunxi_clk_factor_pll_video1,  	&get_factors_pll_video1, &calc_rate_media       ,(struct clk_ops*)NULL},
    //{"pll_gpu",     hosc_parents, 1,          0,          PLL_GPU,    PLL_GPU,     LOCKBIT(28),&sunxi_clk_factor_pll_gpu,   	&get_factors_pll_gpu,    &calc_rate_media     	,(struct clk_ops*)NULL},
    {"pll_mipi",    mipi_parents, 2,          0,          MIPI_PLL,   MIPI_PLL,    LOCKBIT(28),&sunxi_clk_factor_pll_mipi,  	&get_factors_pll_mipi,   &calc_rate_pll_mipi  	,&pll_mipi_ops},
    //{"pll_hsic",    hosc_parents, 1,          0,          PLL_HSIC,   PLL_HSIC,    LOCKBIT(28),&sunxi_clk_factor_pll_hsic,  	&get_factors_pll_hsic,   &calc_rate_media     	,(struct clk_ops*)NULL},
	{"pll_de",      hosc_parents, 1,          0,          PLL_DE,     PLL_DE,      LOCKBIT(28),&sunxi_clk_factor_pll_de,    	&get_factors_pll_de,     &calc_rate_media       ,(struct clk_ops*)NULL},
    //{"pll_ddr1",    hosc_parents, 1,CLK_GET_RATE_NOCACHE, PLL_DDR1,   PLL_DDR1,    LOCKBIT(28),&sunxi_clk_factor_pll_ddr1,   	&get_factors_pll_ddr1,   &calc_rate_pll_ddr1    ,(struct clk_ops*)NULL},
};

static const char *de_parents[] = {"pll_periph0x2", "pll_de", "", "", "", "","",""};
static const char *tcon0_parents[] = {"pll_mipi", "", "pll_video0x2", "", "", "", "", ""};
static const char *tcon1_parents[] = {"pll_video0", "", "pll_video1", ""};
static const char *mipidsi_parents[] = {"pll_video0", "", "pll_periph0",""};
static const char *lvds_parents[] = {"tcon0"};
static const char *hdmi_parents[]= {"pll_video0","pll_video1","",""};

/*
static const char *cpu_parents[] = {"losc", "hosc", "pll_cpu", "pll_cpu"};
static const char *cpuapb_parents[] = {"cpu"};
static const char *axi_parents[] = {"cpu"};
static const char *pll_periphahb0_parents[] = {"pll_periph0"};
static const char *ahb1_parents[] = {"losc", "hosc", "axi", "pll_periphahb0"};
static const char *apb1_parents[] = {"ahb1"};
static const char *apb2_parents[] = {"losc", "hosc", "pll_periph0x2", "pll_periph0x2"};
static const char *ahb2_parents[] = {"ahb1" , "pll_periph0d2" , "" , ""};
static const char *ths_parents[] = {"hosc","","",""};
static const char *periph_parents[] = {"hosc", "pll_periph0","pll_periph1",""};
static const char *periphx2_parents[] = {"hosc", "pll_periph0x2","pll_periph1x2",""};
static const char *ts_parents[] = {"hosc","pll_periph0","","","","","","","","","","","","","",""};
static const char *i2s_parents[] = {"pll_audiox8", "pll_audiox4", "pll_audiox2", "pll_audio"};
static const char *audio_parents[] = {"pll_audio"};
static const char *hoscd2_parents[] = {"hoscd2"};
static const char* hsic_parents[] =  {"pll_hsic"};
static const char *mbus_parents[] = {"hosc", "pll_periph0x2", "pll_ddr0", "pll_ddr1"};
static const char *de_parents[] = {"pll_periph0x2", "pll_de", "", "", "", "","",""};
static const char *tcon0_parents[] = {"pll_mipi", "", "pll_video0x2", "", "", "", "", ""};
static const char *tcon1_parents[] = {"pll_video0", "", "pll_video1", ""};
static const char *periphx_parents[] = {"pll_periph0","pll_periph1","","","","","",""};
static const char *csi_m_parents[] = {"hosc", "pll_video1", "pll_periph1", "", "", "","",""};
static const char *ve_parents[] = {"pll_ve"};
static const char *adda_parents[] = {"pll_audio"};
static const char *addax4_parents[] = {"pll_audiox4"};
static const char *hdmi_parents[]= {"pll_video0","pll_video1","",""};
static const char *mipidsi_parents[] = {"pll_video0", "", "pll_periph0",""};
static const char *gpu_parents[] = {"pll_gpu"};
static const char *lvds_parents[] = {"tcon0"};
static const char *ahb1mod_parents[] = {"ahb1"};
static const char *ahb2mod_parents[] = { "ahb2"};
static const char *apb1mod_parents[] = {"apb1"};
static const char *apb2mod_parents[] = {"apb2"};
static const char *sdram_parents[] = {"pll_ddr0", "pll_ddr1", "",""};
static const char *cpurpll_peri0_parents[] = {"pll_periph0"};
static const char *cpurcpus_parents[] = {"losc" , "hosc" , "cpurpll_peri0" , "iosc" };
static const char *cpurahbs_parents[] = {"cpurcpus"};
static const char *cpurapbs_parents[] = {"cpurahbs"};
static const char *cpurdev_parents[]  = {"losc", "hosc","",""};
static const char *cpurpio_parents[]  = {"cpurapbs"};
static const char *usbohci_parents[] = {"usbohci_16"};
static const char *losc_parents[] = {"losc"};
*/
struct sunxi_clk_comgate com_gates[]={
{"csi",      0,  0x3,    BUS_GATE_SHARE|RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
{"adda",     0,  0x1,    BUS_GATE_SHARE|RST_GATE_SHARE,                 0},
{"usbhci1",   0,  0x3,    RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
{"usbhci0",   0,  0x3,    RST_GATE_SHARE|MBUS_GATE_SHARE, 0},
};

static int clk_lock = 0;
/*
SUNXI_CLK_PERIPH(name,    mux_reg,    mux_shift, mux_width, div_reg,    div_mshift, div_mwidth, div_nshift, div_nwidth, gate_flags, enable_reg, reset_reg, bus_gate_reg, drm_gate_reg, enable_shift, reset_shift, bus_gate_shift, dram_gate_shift, lock,com_gate,com_gate_off)
*/
/*
SUNXI_CLK_PERIPH(cpu,     CPU_CFG,    16,        2,         0,          0,          0,          0,          0,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpuapb,  0,    	   0,        0,         CPU_CFG,    8,          2,          0,          0,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(axi,     0,           0,        0,         CPU_CFG,    0,          2,          0,          0,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(pll_periphahb0,  0,   0,        0,         AHB1_CFG,   6,          2,          0,          0,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(ahb1,    AHB1_CFG,   12,        2,         AHB1_CFG,   0,          0,          4,          2,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(apb1,    0,           0,        0,         AHB1_CFG,   0,          0,          8,          2,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(apb2,    APB2_CFG,   24,        2,         APB2_CFG,   0,          5,         16,          2,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(ahb2,    AHB2_CFG,    0,        2,         0,   		0,          0,          0,          0,          0,          0,          0,         0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(ths,     THS_CFG,    24,        2,         THS_CFG,    0,          0,          0,          2,          0,          THS_CFG,    BUS_RST3,  BUS_GATE2,    0,           31,            8,           8,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(nand,    NAND_CFG,   24,        2,         NAND_CFG,   0,          4,         16,          2,          0,          NAND_CFG,   BUS_RST0,  BUS_GATE0,    0,           31,           13,          13,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_mod,  SD0_CFG,24,        2,         SD0_CFG,    0,          4,         16,          2,          0,          SD0_CFG,    0,  		0,    		 0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_bus,  0,       0,        0,          0,    	    0,          0,          0,          0,          0,          0,    		0,  BUS_GATE0,    		 0,            0,            0,           8,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc0_rst,  0,       0,        0,          0,         0,          0,          0,          0,          0,          0,   BUS_RST0,  		0,    		 0,            0,            8,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_mod, SD1_CFG, 24,        2,         SD1_CFG,    0,          4,         16,          2,          0,          SD1_CFG,    0,  		0,    		 0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_bus,  0,       0,        0,         0,    		0,          0,          0,          0,          0,          0,    		0,  BUS_GATE0,    		 0,            0,            0,           9,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc1_rst,  0,       0,        0,         0,    		0,          0,          0,          0,          0,          0,    BUS_RST0,   		0,    		 0,            0,            9,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_mod, SD2_CFG, 24,        2,         SD2_CFG,    0,          4,         16,          2,          0,          SD2_CFG,    0,  		0,    		 0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_bus,  0,       0,        0,         0,    		0,          0,          0,          0,          0,          0,          0,  BUS_GATE0,    		 0,            0,            0,          10,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdmmc2_rst,  0,       0,        0,         0,    		0,          0,          0,          0,          0,          0,    BUS_RST0,  		0,    		 0,            0,           10,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(ts,      TS_CFG,     24,        4,         TS_CFG,     0,          4,         16,          2,          0,          TS_CFG,     BUS_RST0,  BUS_GATE0,    DRAM_GATE,   31,           18,          18,              3,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(ce,      CE_CFG,     24,        2,         CE_CFG,     0,          4,         16,          2,          0,          CE_CFG,   	BUS_RST0,  BUS_GATE0,    0,           31,            5,           5,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(spi0,    SPI0_CFG,   24,        2,         SPI0_CFG,   0,          4,         16,          2,          0,          SPI0_CFG,   BUS_RST0,  BUS_GATE0,    0,           31,           20,          20,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(spi1,    SPI1_CFG,   24,        2,         SPI1_CFG,   0,          4,         16,          2,          0,          SPI1_CFG,   BUS_RST0,  BUS_GATE0,    0,           31,           21,          21,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(i2s0, 	  I2S0_CFG,	  16,        2,         0,   	    0,          0,          0,          0,          0,          I2S0_CFG,	BUS_RST3,  BUS_GATE2,    0,           31,           12,          12,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(i2s1,    I2S1_CFG,   16,        2,         0,   	    0,          0,          0,          0,          0,          I2S1_CFG,	BUS_RST3,  BUS_GATE2,    0,           31,           13,          13,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(i2s2,    I2S2_CFG,   16,        2,         0,   	    0,          0,          0,          0,          0,          I2S2_CFG,	BUS_RST3,  BUS_GATE2,    0,           31,           14,          14,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(spdif,   0,           0,        0,         SPDIF_CFG,  0,          4,          0,          0,          0,          SPDIF_CFG,  BUS_RST3,  BUS_GATE2,    0,           31,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbphy0, 0,           0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    USB_CFG,   0,            0,            8,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbphy1, 0,           0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    USB_CFG,   0,            0,            9,            1,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbhsic, 0,           0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    USB_CFG,   0,            0,           10,            2,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbhsic12m,0,         0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    0,         0,            0,           11,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbohci_16,0,         0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    0,  	   0,			 0,   	  	  16,            0,           0,              0,               &clk_lock,NULL,    		   0);
SUNXI_CLK_PERIPH(usbohci1, 0,          0,        0,         0,          0,          0,          0,          0,          0,          USB_CFG,    BUS_RST0,  BUS_GATE0,BUS_RST0,   	  17,           29,          29,             25,               &clk_lock,&com_gates[2],    0);
SUNXI_CLK_PERIPH(usbohci0, 0,          0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,BUS_RST0,   	   0,           28,          28,             24,               &clk_lock,&com_gates[3],    0);
SUNXI_CLK_PERIPH(usbehci1, 0,          0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,BUS_RST0,   	   0,           29,          25,             25,               &clk_lock,&com_gates[2],    1);
SUNXI_CLK_PERIPH(usbehci0, 0,          0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,BUS_RST0,   	   0,           28,          24,             24,               &clk_lock,&com_gates[3],    1);
SUNXI_CLK_PERIPH(de,      DE_CFG,     24,        3,         DE_CFG,     0,          4,          0,          0,          0,          DE_CFG,     BUS_RST1,  BUS_GATE1,    0,           31,           12,          12,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(tcon0,   TCON0_CFG,  24,        3,         0,  		0,          0,          0,          0,          0,          TCON0_CFG,  BUS_RST1,  BUS_GATE1,    0,           31,            3,           3,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(tcon1,   TCON1_CFG,  24,        2,         TCON1_CFG,  0,          4,          0,          0,          0,          TCON1_CFG,  BUS_RST1,  BUS_GATE1,	 0,           31,            4,           4,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(deinterlace,DEINTERLACE_CFG,24, 3, DEINTERLACE_CFG,    0,          4,          0,          0,          0,  DEINTERLACE_CFG,    BUS_RST1,  BUS_GATE1,    DRAM_GATE,   31,            5,           5,              2,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(csi_s,   CSI_CFG,    24,        3,         CSI_CFG,   16,          4,          0,          0,          0,          CSI_CFG,    BUS_RST1,  BUS_GATE1,    DRAM_GATE,   31,            8,           8,              1,               &clk_lock,&com_gates[0],    0);
SUNXI_CLK_PERIPH(csi_m,   CSI_CFG,     8,        3,         CSI_CFG,    0,          5,          0,          0,          0,          CSI_CFG,    BUS_RST1,  BUS_GATE1,    DRAM_GATE,   15,            8,           8,              1,               &clk_lock,&com_gates[0],    1);
SUNXI_CLK_PERIPH(csi_misc, 0,    	   0,        0,         0,          0,          0,          0,          0,          0,          CSI_MISC,   BUS_RST1,  BUS_GATE1,    DRAM_GATE,   31,            8,           8,              1,               &clk_lock,&com_gates[0],    2);
SUNXI_CLK_PERIPH(ve,       0,          0,        0,         VE_CFG,    16,          3,          0,          0,          0,          VE_CFG,     BUS_RST1,  BUS_GATE1,    DRAM_GATE,   31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(adda,     0,          0,        0,         0,          0,          0,          0,          0,          0,          ADDA_CFG,   BUS_RST3,  BUS_GATE2,    0,           31,            0,           0,              0,               &clk_lock,&com_gates[1],    0);
SUNXI_CLK_PERIPH(addax4,   0,          0,        0,         0,          0,          0,          0,          0,          0,          ADDA_CFG,   BUS_RST3,  BUS_GATE2,    0,           30,            0,           0,              0,               &clk_lock,&com_gates[1],    1);
SUNXI_CLK_PERIPH(avs,      0,          0,        0,         0,          0,          0,          0,          0,          0,          AVS_CFG,    0,         0,            0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi,	   HDMI_CFG,  24,        2,         HDMI_CFG,   0,          4,          0,          0,          0,          HDMI_CFG,   BUS_RST1,  BUS_GATE1,    BUS_RST1,    31,           11,          11,             10,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi_slow,0,  		   0,        0,         0,          0,          0,          0,          0,          0,          HDMI_SLOW,  0,          0,           0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(mbus,    MBUS_CFG,   24,        2,         MBUS_CFG,   0,          3,          0,          0,          0,          MBUS_CFG,   MBUS_RST,   0,           0,           31,           31,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(mipidsi, MIPI_DSI,    8,        2,         MIPI_DSI,   0,          4,          0,          0,          0,          MIPI_DSI,   BUS_RST0,  BUS_GATE0,    0,           15,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(gpu,      0,          0,        0,         GPU_CFG,    0,          3,          0,          0,          0,          GPU_CFG,    BUS_RST1,  BUS_GATE1,    0,           31,           20,          20,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(usbotg,   0,          0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,    0,            0,           23,          23,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(gmac,     0,          0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,    0,            0,           17,          17,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(sdram,	  DRAM_CFG,   20,        2,         DRAM_CFG,   0,          2,          0,          0,          0,          DRAM_CFG,   BUS_RST0,  BUS_GATE0,    0,           31,           14,          14,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(dma,      0,    	   0,        0,         0,   		0,          0,          0,          0,          0,          0,          BUS_RST0,  BUS_GATE0,    0,            0,            6,           6,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hwspinlock_rst, 0,    0,        0,         0,          0,          0,           0,         0,          0,          0,          BUS_RST1,   	   0,    0,            0,           22,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hwspinlock_bus, 0,    0,        0,         0,          0,          0,           0,         0,          0,          0,                 0,  BUS_GATE1,    0,            0,            0,          22,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(msgbox,   0,          0,        0,         0,          0,          0,           0,         0,          0,          0,          BUS_RST1,  BUS_GATE1,    0,            0,           21,          21,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(lvds,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST2,  0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(uart0,   0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,           16,          16,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(uart1,   0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,           17,          17,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(uart2,   0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,           18,          18,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(uart3,   0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,           19,          19,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(uart4,   0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,           20,          20,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(scr,     0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,            5,           5,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(twi0,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(twi1,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(twi2,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,            2,           2,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(twi3,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST4,   BUS_GATE3,   0,            0,            3,           3,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(pio,     0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          	   0,   BUS_GATE2,   0,            0,            0,           5,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurcir,  CPUS_CIR,  24,        2,  CPUS_CIR,   		0,          4,         16,          2,          0,   CPUS_CIR,    CPUS_APB0_GATE,CPUS_APB0_RST,  0,       	  31,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurpll_peri0,  0,    0,        0,  CPUS_CFG,   		8,          5,          0,          0,          0,          0,          	   0,         	0,   0,            0,            0,           0,              0,			   &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurcpus, CPUS_CFG,  16,        2,  CPUS_CFG,   		0,          0,          4,          2,          0,          0,				   0,  			0,   0,     	   0,            0,           0,              0,			   &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurahbs, 0,          0,        0,   		0,          0,          0,          0,          0,          0,          0,				   0,  			0,   0,     	   0,            0,           0,              0,			   &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurapbs, 0,          0,        0, CPUS_APB0,  		0,          2,          0,          0,          0,          0,				   0,  			0, 	 0,     	   0,            0,           0,              0,			   &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(cpurpio,  0,  		   0,        0,         0,   		0,          0,          0,          0,          0,          0,    CPUS_APB0_GATE,			0,   0,       	   0,            0,           0,              0,			   &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(losc_out, 0,          0,        0,         0,          0,          0,          0,          0,          0,          0,                 0, LOSC_OUT_GATE, 0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
*/
SUNXI_CLK_PERIPH(de,	  DE_CFG,	  24,		 3, 		DE_CFG, 	0,			4,			0,			0,			0,			DE_CFG, 	BUS_RST1,  BUS_GATE1,	 0, 		  31,			12, 		 12,			  0,			   &clk_lock,NULL,			   0);
SUNXI_CLK_PERIPH(tcon0,   TCON0_CFG,  24,		 3, 		0,			0,			0,			0,			0,			0,			TCON0_CFG,	BUS_RST1,  BUS_GATE1,	 0, 		  31,			 3, 		  3,			  0,			   &clk_lock,NULL,			   0);
SUNXI_CLK_PERIPH(tcon1,   TCON1_CFG,  24,		 2, 		TCON1_CFG,	0,			4,			0,			0,			0,			TCON1_CFG,	BUS_RST1,  BUS_GATE1,	 0, 		  31,			 4, 		  4,			  0,			   &clk_lock,NULL,			   0);
SUNXI_CLK_PERIPH(lvds,    0,           0,        0,         0,          0,          0,          0,          0,          0,          0,          BUS_RST2,  0,            0,            0,            0,           0,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(mipidsi, MIPI_DSI,    8,        2,         MIPI_DSI,   0,          4,          0,          0,          0,          MIPI_DSI,   BUS_RST0,  BUS_GATE0,    0,           15,            1,           1,              0,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi,	   HDMI_CFG,  24,        2,         HDMI_CFG,   0,          4,          0,          0,          0,          HDMI_CFG,   BUS_RST1,  BUS_GATE1,    BUS_RST1,    31,           11,          11,             10,               &clk_lock,NULL,             0);
SUNXI_CLK_PERIPH(hdmi_slow,0,  		   0,        0,         0,          0,          0,          0,          0,          0,          HDMI_SLOW,  0,          0,           0,           31,            0,           0,              0,               &clk_lock,NULL,             0);


struct periph_init_data sunxi_periphs_init[] = {
	{"de",       0,       				de_parents,  	  ARRAY_SIZE(de_parents),  		&sunxi_clk_periph_de},
    {"tcon0",    0,       				tcon0_parents,    ARRAY_SIZE(tcon0_parents),    &sunxi_clk_periph_tcon0},
    {"tcon1",    0,       				tcon1_parents,    ARRAY_SIZE(tcon1_parents),    &sunxi_clk_periph_tcon1},
    {"hdmi",     0,        			    hdmi_parents,     ARRAY_SIZE(hdmi_parents),     &sunxi_clk_periph_hdmi},
    {"hdmi_slow",0,       			    hosc_parents,     ARRAY_SIZE(hosc_parents),     &sunxi_clk_periph_hdmi_slow},
	{"lvds",     0,        			    lvds_parents,     ARRAY_SIZE(lvds_parents),     &sunxi_clk_periph_lvds},
	{"mipidsi",  0,       				mipidsi_parents,  ARRAY_SIZE(mipidsi_parents),  &sunxi_clk_periph_mipidsi},
};

static void  *sunxi_clk_base = NULL;

void  init_clocks(void)
{
    int     i;
    //struct clk *clk;
    struct factor_init_data *factor;
    struct periph_init_data *periph;

    /* get clk register base address */
    sunxi_clk_base = (void*)0x01c20000; //fixed base address.
    sunxi_clk_factor_initlimits();

	sunxi_clk_get_factors_ops(&pll_mipi_ops);
	pll_mipi_ops.get_parent = get_parent_pll_mipi;
	pll_mipi_ops.set_parent = set_parent_pll_mipi;
	pll_mipi_ops.enable = clk_enable_pll_mipi;
	pll_mipi_ops.disable = clk_disable_pll_mipi;

    clk_register_fixed_rate(NULL, "hosc", NULL, CLK_IS_ROOT, 24000000);

    /* register normal factors, based on sunxi factor framework */
    for(i=0; i<ARRAY_SIZE(sunxi_factos); i++) {
        factor = &sunxi_factos[i];
        factor->priv_regops = NULL;
        sunxi_clk_register_factors(NULL, (void*)sunxi_clk_base, (struct factor_init_data*)factor);
	}

    /* register periph clock */
    for(i=0; i<ARRAY_SIZE(sunxi_periphs_init); i++) {
        periph = &sunxi_periphs_init[i];
        periph->periph->priv_regops = NULL;
		sunxi_clk_register_periph(periph->name, periph->parent_names,
					periph->num_parents,periph->flags, sunxi_clk_base, periph->periph);
    }
	printf("%s: finish init_clocks.\n",__func__);
}

