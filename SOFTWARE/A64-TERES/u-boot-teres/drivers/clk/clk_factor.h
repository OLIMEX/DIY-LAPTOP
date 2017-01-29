
#ifndef __MACH_SUNXI_CLK_FACTORS_H
#define __MACH_SUNXI_CLK_FACTORS_H
#include<clk/clk_plat.h>


struct clk_factors_value {
    u16 factorn;
    u16 factork;

    u16 factorm;
    u16 factorp;

    u16 factord1;
    u16 factord2;

    u16 frac_mode;
    u16 frac_freq;
};


/**
 * struct sunxi_clk_factors_config - factor config
 *
 * @nshift:     shift to factor-n bit field
 * @nwidth:     width of factor-n bit field
 * @kshift:     shift to factor-k bit field
 * @kwidth:     width of factor-k bit field
 * @mshift:     shift to factor-m bit field
 * @mwidth:     width of factor-m bit field
 * @pshift:     shift to factor-p bit field
 * @pwidth:     width of factor-p bit field
 * @d1shift:    shift to factor-d1 bit field
 * @d1width:    width of factor-d1 bit field
 * @d2shift:    shift to factor-d2 bit field
 * @d2width:    width of factor-d2 bit field
 * @frac:       flag of fraction
 * @outshift:   shift to frequency select bit field
 * @modeshift:  shift to fraction/integer mode select
 * @enshift:    shift to factor enable bit field
 * @lockshift:  shift to factor lock status bit filed
 * @sdmshift:   shift to factor sdm enable bit filed
 * @sdmwidth    shift to factor sdm width bit filed
 * @sdmpat      sdmpat reg address offset
 * @sdmval      sdm default value
 * @updshift	shift to update bit (especial for ddr/ddr0/ddr1)
 */
struct sunxi_clk_factors_config {
    u8 nshift;
    u8 nwidth;
    u8 kshift;
    u8 kwidth;

    u8 mshift;
    u8 mwidth;
    u8 pshift;
    u8 pwidth;

    u8 d1shift;
    u8 d1width;
    u8 d2shift;
    u8 d2width;

    u8 frac;
    u8 outshift;
    u8 modeshift;
    u8 enshift;
    
    u8 lockshift;
    u8 sdmshift;
    u8 sdmwidth;

    u32 sdmpat;
    u32 sdmval;

	u32 updshift;
	u32 delay;
};
struct sunxi_clk_factor_freq{
	u32		factor;	
    u32   freq;	
};

/**
 * struct sunxi_clk_factors - factor clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @dev:        device handle who register this clock
 * @name:       name of the clock
 * @parent_name:name of the parent
 * @reg:        register address for the factor
 * @config:     configuration of the factor
 * @get_factor: function for get factors parameter under a given frequency
 * @calc_rate:  function for calculate the factor frequency
 * @lock:       register lock
 * @flags:      factor optimal configurations
 *
 */

struct factor_init_data {
    const char          *name;
    const char          **parent_names;
    int                 num_parents;
    unsigned long       flags;
    unsigned long       reg;
    unsigned long       lock_reg;
    unsigned char       lock_bit;    
    struct sunxi_clk_factors_config *config;
    int (*get_factors) (u32 rate, u32 parent_rate, struct clk_factors_value *factor);
    unsigned long (*calc_rate) (u32 parent_rate, struct clk_factors_value *factor);
    struct clk_ops      * priv_ops;
    void				*priv_regops;
}; 
struct sunxi_clk_factors {
    struct clk_hw       hw;
    unsigned long       flags;
    void         		*reg;
    void				*lock_reg;
    unsigned char       lock_bit;    
    struct sunxi_clk_factors_config *config;
    int (*get_factors) (u32 rate, u32 parent_rate, struct clk_factors_value *factor);
    unsigned long (*calc_rate) (u32 parent_rate, struct clk_factors_value *factor);
    struct sunxi_reg_ops*		priv_regops;
};


struct sunxi_clk_pat_item
{
    char* name;
    char* patname;
};
static inline u32 factor_readl(struct sunxi_clk_factors * factor, void * reg)
{
    //return (((unsigned int)factor->priv_regops)?factor->priv_regops->reg_readl(reg):readl(reg));
    return (readl(reg));
}
static inline void factor_writel(struct sunxi_clk_factors * factor, unsigned int val, void * reg)
{
    //(((unsigned int)factor->priv_regops)?factor->priv_regops->reg_writel(val,reg):writel(val,reg));
    writel(val,reg);
}
void sunxi_clk_get_factors_ops(struct clk_ops* ops);
int sunxi_clk_register_factors(void *pfactor,void  *base,struct factor_init_data* init_data);


#define SUNXI_CLK_FACTORS(name, _nshift, _nwidth, _kshift, _kwidth, _mshift, _mwidth,   \
                  _pshift, _pwidth, _d1shift, _d1width, _d2shift, _d2width,     \
                  _frac, _outshift, _modeshift, _enshift, _sdmshift, _sdmwidth, _sdmpat, _sdmval)     \
    static struct sunxi_clk_factors_config sunxi_clk_factor_##name ={            \
        .nshift = _nshift,  \
        .nwidth = _nwidth,  \
        .kshift = _kshift,  \
        .kwidth = _kwidth,  \
        .mshift = _mshift,  \
        .mwidth = _mwidth,  \
        .pshift = _pshift,  \
        .pwidth = _pwidth,  \
        .d1shift = _d1shift,    \
        .d1width = _d1width,    \
        .d2shift = _d2shift,    \
        .d2width = _d2width,    \
        .frac = _frac,  \
        .outshift = _outshift,  \
        .modeshift =_modeshift,     \
        .enshift =_enshift,    \
	.sdmshift=_sdmshift,	\
	.sdmwidth=_sdmwidth,	\
	.sdmpat  =_sdmpat,	\
	.sdmval  =_sdmval,	\
	.updshift = 0\
	}
    #define FACTOR_ALL(nv,ns,nw,kv,ks,kw,mv,ms,mw, \
									pv,ps,pw,d0v,d0s,d0w,d1v,d1s,d1w) \
									((nv&((1<<nw)-1))<<ns|(kv&((1<<kw)-1))<<ks| \
									(mv&((1<<mw)-1))<<ms|(pv&((1<<pw)-1))<<ps| \
									(d0v&((1<<d0w)-1))<<d0s|(d1v&((1<<d1w)-1))<<d1s)

#define SUNXI_CLK_FACTORS_UPDATE(name, _nshift, _nwidth, _kshift, _kwidth, _mshift, _mwidth,   \
                  _pshift, _pwidth, _d1shift, _d1width, _d2shift, _d2width,     \
                  _frac, _outshift, _modeshift, _enshift, _sdmshift, _sdmwidth, _sdmpat, _sdmval , _updshift)     \
    static struct sunxi_clk_factors_config sunxi_clk_factor_##name ={            \
        .nshift = _nshift,  \
        .nwidth = _nwidth,  \
        .kshift = _kshift,  \
        .kwidth = _kwidth,  \
        .mshift = _mshift,  \
        .mwidth = _mwidth,  \
        .pshift = _pshift,  \
        .pwidth = _pwidth,  \
        .d1shift = _d1shift,    \
        .d1width = _d1width,    \
        .d2shift = _d2shift,    \
        .d2width = _d2width,    \
        .frac = _frac,  \
        .outshift = _outshift,  \
        .modeshift =_modeshift,     \
        .enshift =_enshift,    \
	.sdmshift=_sdmshift,	\
	.sdmwidth=_sdmwidth,	\
	.sdmpat  =_sdmpat,	\
	.sdmval  =_sdmval,	\
	.updshift = _updshift\
	}

#define SUNXI_CLK_FACTORS_DELAY(name, _nshift, _nwidth, _kshift, _kwidth, _mshift, _mwidth,   \
                  _pshift, _pwidth, _d1shift, _d1width, _d2shift, _d2width,     \
                  _frac, _outshift, _modeshift, _enshift, _sdmshift, _sdmwidth, _sdmpat, _sdmval , _delay)     \
    static struct sunxi_clk_factors_config sunxi_clk_factor_##name ={            \
        .nshift = _nshift,  \
        .nwidth = _nwidth,  \
        .kshift = _kshift,  \
        .kwidth = _kwidth,  \
        .mshift = _mshift,  \
        .mwidth = _mwidth,  \
        .pshift = _pshift,  \
        .pwidth = _pwidth,  \
        .d1shift = _d1shift,    \
        .d1width = _d1width,    \
        .d2shift = _d2shift,    \
        .d2width = _d2width,    \
        .frac = _frac,  \
        .outshift = _outshift,  \
        .modeshift =_modeshift,     \
        .enshift =_enshift,    \
	.sdmshift=_sdmshift,	\
	.sdmwidth=_sdmwidth,	\
	.sdmpat  =_sdmpat,	\
	.sdmval  =_sdmval,	\
	.delay = _delay\
	}

int sunxi_clk_get_common_factors(struct sunxi_clk_factors_config* f_config,struct clk_factors_value *factor, struct sunxi_clk_factor_freq table[],unsigned long index,unsigned long tbl_size);
int sunxi_clk_get_common_factors_search(struct sunxi_clk_factors_config* f_config,struct clk_factors_value *factor, struct sunxi_clk_factor_freq table[],unsigned long index,unsigned long tbl_count);

#endif
