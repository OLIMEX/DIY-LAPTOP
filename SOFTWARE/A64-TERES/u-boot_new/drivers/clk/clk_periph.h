#ifndef __MACH_SUNXI_CLK_PERIPH_H
#define __MACH_SUNXI_CLK_PERIPH_H

#include<clk/clk_plat.h>
#include<clk/clk.h>


/**
 * struct sunxi_clk_periph_gate - peripheral gate clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @flags:      hardware-specific flags
 * @enable:     enable register
 * @reset:      reset register
 * @bus:        bus gating resiter
 * @dram:       dram gating register
 * @enb_shift:  enable gate bit shift
 * @rst_shift:  reset gate bit shift
 * @bus_shift:  bus gate bit shift
 * @ddr_shift:  dram gate bit shift
 *
 * Flags:
 * SUNXI_PERIPH_NO_GATE - this flag indicates that module gate is not allowed for this module.
 * SUNXI_PERIPH_NO_RESET - This flag indicates that reset is not allowed for this module.
 * SUNXI_PERIPH_NO_BUS_GATE - This flag indicates that bus gate is not allowed for this module.
 * SUNXI_PERIPH_NO_DDR_GATE - This flag indicates that dram gate is not allowed for this module.
 */
struct sunxi_clk_periph_gate {
    u32             flags;
    void     		*enable;
    void     		*reset;
    void     		*bus;
    void     		*dram;
    u8              enb_shift;
    u8              rst_shift;
    u8              bus_shift;
    u8              ddr_shift;
};

/**
 * struct sunxi_clk_periph_div - periph divider clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @reg:        register containing divider
 * @flags:      hardware-specific flags
 * @mshift:     shift to the divider-m bit field, div = (m+1)
 * @mwidth:     width of the divider-m bit field
 * @nshift:     shift to the divider-n bit field, div = (1<<n)
 * @nwidth:     width of the divider-n bit field
 * @lock:       register lock
 *
 * Flags:
 */
struct sunxi_clk_periph_div {
    void     		*reg;
    u8              mshift;
    u8              mwidth;
    u8              nshift;
    u8              nwidth;
};


/**
 * struct sunxi_clk_periph_mux - multiplexer clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @reg:        register controlling multiplexer
 * @shift:      shift to multiplexer bit field
 * @width:      width of mutliplexer bit field
 * @lock:       register lock
 *
 * Clock with multiple selectable parents.  Implements .get_parent, .set_parent
 * and .recalc_rate
 *
 */
struct sunxi_clk_periph_mux {
    void     		*reg;
    u8              shift;
    u8              width;
};

struct sunxi_clk_comgate {
    const char* name;
    u8 val;
    u8 mask;
    u8 share;
    u8 res;
};

#define BUS_GATE_SHARE  0x01
#define RST_GATE_SHARE  0x02
#define MBUS_GATE_SHARE 0x04
#define MOD_GATE_SHARE  0x08

#define IS_SHARE_BUS_GATE(x)  (x->com_gate?((x->com_gate->share & BUS_GATE_SHARE)?1:0):0)
#define IS_SHARE_RST_GATE(x)  (x->com_gate?((x->com_gate->share & RST_GATE_SHARE)?1:0):0)
#define IS_SHARE_MBUS_GATE(x) (x->com_gate?((x->com_gate->share & MBUS_GATE_SHARE)?1:0):0)
#define IS_SHARE_MOD_GATE(x)  (x->com_gate?((x->com_gate->share & MOD_GATE_SHARE)?1:0):0)

/**
 * struct sunxi-clk-periph - peripheral clock
 *
 * @hw:         handle between common and hardware-specific interfaces
 * @mux:        mux clock
 * @divider:    divider clock
 * @gate:       gate clock
 * @mux_ops:    mux clock ops
 * @div_ops:    divider clock ops
 * @gate_ops:   gate clock ops
 */

struct sunxi_clk_periph {
    struct clk_hw            		hw;
    unsigned long                   flags;
    void                           *lock;

    struct sunxi_clk_periph_mux     mux;
    struct sunxi_clk_periph_gate    gate;
    struct sunxi_clk_periph_div     divider;
    struct sunxi_clk_comgate*       com_gate;
    u8                              com_gate_off;
    struct clk_ops           		*priv_clkops;
    void				  			*priv_regops;
};

struct periph_init_data {
    const char          *name;
    unsigned long       flags;        
    const char          **parent_names;
    int                 num_parents;
    struct sunxi_clk_periph *periph;
};


static inline u32 periph_readl(struct sunxi_clk_periph * periph, void  * reg)
{
    //return (((unsigned int)periph->priv_regops)?periph->priv_regops->reg_readl(reg):readl(reg));
	return (readl(reg));
}
static inline void periph_writel(struct sunxi_clk_periph * periph, unsigned int val, void  * reg)
{
    //(((unsigned int)periph->priv_regops)?periph->priv_regops->reg_writel(val,reg):writel(val,reg));
    writel(val,reg);
}
int sunxi_clk_register_periph(const char *name, const char **parent_names,
            int num_parents, unsigned long flags, void  *base, struct sunxi_clk_periph *periph);

extern void sunxi_clk_get_periph_ops(struct clk_ops* ops);
#define to_clk_periph(_hw) container_of(_hw, struct sunxi_clk_periph, hw)


#define SUNXI_CLK_PERIPH(name, _mux_reg, _mux_shift, _mux_width,  \
            _div_reg, _div_mshift, _div_mwidth, _div_nshift, _div_nwidth,   \
            _gate_flags, _enable_reg, _reset_reg, _bus_gate_reg, _drm_gate_reg, \
            _enable_shift, _reset_shift, _bus_gate_shift, _dram_gate_shift, _lock,_com_gate,_com_gate_off) \
static struct sunxi_clk_periph sunxi_clk_periph_##name ={       \
        .lock = _lock,                                          \
                                                                \
        .mux = {                                                \
            .reg = (void*)_mux_reg,                   \
            .shift = _mux_shift,                                \
            .width = _mux_width,                                \
        },                                                      \
                                                                \
        .divider = {                                            \
            .reg = (void*)_div_reg,                   \
            .mshift = _div_mshift,                              \
            .mwidth = _div_mwidth,                              \
            .nshift = _div_nshift,                              \
            .nwidth = _div_nwidth,                              \
        },                                                      \
        .gate = {                                               \
            .flags = _gate_flags,                               \
            .enable = (void*)_enable_reg,             \
            .reset = (void*)_reset_reg,               \
            .bus = (void*)_bus_gate_reg,              \
            .dram = (void*)_drm_gate_reg,             \
            .enb_shift = _enable_shift,                         \
            .rst_shift = _reset_shift,                          \
            .bus_shift = _bus_gate_shift,                       \
            .ddr_shift = _dram_gate_shift,                      \
        },                                                      \
        .com_gate = _com_gate,                                          \
        .com_gate_off = _com_gate_off,                                          \
    }


#endif
