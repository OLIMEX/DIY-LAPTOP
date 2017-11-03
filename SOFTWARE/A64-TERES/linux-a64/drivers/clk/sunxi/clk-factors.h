/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable factor-based clock implementation
 */
#ifndef __MACH_SUNXI_CLK_FACTORS_H
#define __MACH_SUNXI_CLK_FACTORS_H

#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/io.h>
#include "clk-sunxi.h"

typedef enum pll_lock_mode
{
	PLL_LOCK_NEW_MODE = 0x0,
	PLL_LOCK_OLD_MODE,
	PLL_LOCK_NONE_MODE,
	PLL_LOCK_MODE_MAX,
} pll_lock_mode_e;

/**
 * struct clk_factors_value - factor value
 *
 * @factorn:    factor-n value
 * @factork:    factor-k value
 * @factorm:    factor-m value
 * @factorp:    factor-p value
 * @factord1:   factor-d1 value
 * @factord2:   factor-d2 value
 * @frac_mode:  fraction mode value
 * @frac_freq:  fraction frequnecy value
 */
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
 * @updshift    shift to update bit (especial for ddr/ddr0/ddr1)
 * @delay       for flat factors delay.
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

	unsigned long sdmpat;
	u32 sdmval;

	u32 updshift;
	u32 delay;
};

struct sunxi_clk_factor_freq{
	u32 factor;
	u32 freq;
};

/**
 * struct factor_init_data - factor init data
 *
 * @name:       name of the clock
 * @parent_name:name of the parent
 * @num_parents:counter of the parents
 * @flags:      factor optimal configurations
 * @reg:        register address for the factor
 * @lock_reg:   register address for check if the pll has locked
 * @lock_bit:   bit offset of the lock_reg, to check if the the pll has locked
 * @pll_lock_ctrl_reg: pll lock control register, this function is first used on
 *              the sun50i, to enable the function of pll hardlock
 * @lock_en_bit:bit offset of the pll_lock_ctrl_reg, to enable the function
 * @config:     configuration of the factor
 * @get_factors:function for get factors parameter under a given frequency
 * @calc_rate:  function for calculate the factor frequency
 * @priv_ops:   private operations hook for the special factor
 * @priv_regops:register operation hook for read/write the register
 *
 */
struct factor_init_data {
	const char          *name;
	const char          **parent_names;
	int                 num_parents;
	unsigned long       flags;
	u64                 reg;
	u64                 lock_reg;
	unsigned char       lock_bit;
	u64                 pll_lock_ctrl_reg;
	unsigned char       lock_en_bit;
	pll_lock_mode_e     lock_mode;
	struct sunxi_clk_factors_config *config;
	int (*get_factors) (u32 rate, u32 parent_rate, struct clk_factors_value *factor);
	unsigned long (*calc_rate) (u32 parent_rate, struct clk_factors_value *factor);
	struct clk_ops * priv_ops;
	struct sunxi_reg_ops*  priv_regops;
};
/**
 * struct sunxi_clk_factors - factor clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @dev:        device handle who register this clock
 * @flags:      factor optimal configurations
 * @reg:        register address for the factor
 * @lock_reg:   register address for check if the pll has locked
 * @lock_bit:   bit offset of the lock_reg, to check if the the pll has locked
 * @pll_lock_ctrl_reg: pll lock control register, this function is first used on
 *              the sun50i, to enable the function of pll hardlock
 * @lock_en_bit:bit offset of the pll_lock_ctrl_reg, to enable the function
 * @get_factor: function for get factors parameter under a given frequency
 * @calc_rate:  function for calculate the factor frequency
 * @lock:       lock for protecting the factors operations
 * @priv_ops:   private operations hook for the special factor
 *
 */
struct sunxi_clk_factors {
	struct clk_hw       hw;
	struct device       *dev;
	unsigned long       flags;
	void __iomem        *reg;
	void __iomem        *lock_reg;
	unsigned char       lock_bit;
	void __iomem        *pll_lock_ctrl_reg;
	unsigned char       lock_en_bit;
	pll_lock_mode_e     lock_mode;
	struct sunxi_clk_factors_config *config;
	int (*get_factors) (u32 rate, u32 parent_rate, struct clk_factors_value *factor);
	unsigned long (*calc_rate) (u32 parent_rate, struct clk_factors_value *factor);
	spinlock_t *lock;
	struct sunxi_reg_ops*  priv_regops;
};
struct sunxi_clk_pat_item
{
	char* name;
	char* patname;
};
static inline u32 factor_readl(struct sunxi_clk_factors * factor, void __iomem * reg)
{
	return (((unsigned long*)factor->priv_regops)?factor->priv_regops->reg_readl(reg):readl(reg));
}
static inline void factor_writel(struct sunxi_clk_factors * factor, unsigned int val, void __iomem * reg)
{
	(((unsigned long*)factor->priv_regops)?factor->priv_regops->reg_writel(val,reg):writel(val,reg));
}
void sunxi_clk_get_factors_ops(struct clk_ops* ops);
struct clk *sunxi_clk_register_factors(struct device *dev,void __iomem *base,spinlock_t *lock,struct factor_init_data* init_data);


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
		.sdmshift=_sdmshift,    \
		.sdmwidth=_sdmwidth,    \
		.sdmpat  =_sdmpat,    \
		.sdmval  =_sdmval,    \
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
		.sdmshift=_sdmshift,    \
		.sdmwidth=_sdmwidth,    \
		.sdmpat  =_sdmpat,    \
		.sdmval  =_sdmval,    \
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
		.sdmshift=_sdmshift,    \
		.sdmwidth=_sdmwidth,    \
		.sdmpat  =_sdmpat,    \
		.sdmval  =_sdmval,    \
		.delay = _delay\
	}

int sunxi_clk_get_common_factors(struct sunxi_clk_factors_config* f_config,struct clk_factors_value *factor, struct sunxi_clk_factor_freq table[],unsigned long index,unsigned long tbl_size);
int sunxi_clk_get_common_factors_search(struct sunxi_clk_factors_config* f_config,struct clk_factors_value *factor, struct sunxi_clk_factor_freq table[],unsigned long index,unsigned long tbl_count);
#endif
