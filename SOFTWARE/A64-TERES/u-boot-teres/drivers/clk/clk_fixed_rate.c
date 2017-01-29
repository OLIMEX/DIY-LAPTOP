#include<clk/clk_plat.h>
#include<clk/clk.h>


/*
 * DOC: basic fixed-rate clock that cannot gate
 *
 * Traits of this clock:
 * prepare - clk_(un)prepare only ensures parents are prepared
 * enable - clk_enable only ensures parents are enabled
 * rate - rate is always a fixed value.  No clk_set_rate support
 * parent - fixed parent.  No clk_set_parent support
 */

#define to_clk_fixed_rate(_hw) container_of(_hw, struct clk_fixed_rate, hw)

static unsigned long clk_fixed_rate_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	return to_clk_fixed_rate(hw)->fixed_rate;
}

struct clk_ops clk_fixed_rate_ops = {
	.recalc_rate = clk_fixed_rate_recalc_rate,
};

struct clk *clk_register_fixed_rate(void *dev, const char *name,
		const char *parent_name, unsigned long flags,
		unsigned long fixed_rate)
{
	struct clk_fixed_rate *fixed;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate fixed-rate clock */
	fixed = malloc(sizeof(struct clk_fixed_rate));
	if (fixed)
		memset(fixed,0,sizeof(struct clk_fixed_rate));
    else {
        printf("%s: could not allocate fixed rate clk\n", __func__);
        return NULL;
    }

	init.name = name;
	init.ops = &clk_fixed_rate_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = (parent_name ? &parent_name: NULL);
	init.num_parents = (parent_name ? 1 : 0);
	/* struct clk_fixed_rate assignments */
	fixed->fixed_rate = fixed_rate;
	fixed->hw.init = &init;

	/* register the clock */
	clk = clk_register(&fixed->hw);

	if (NULL == clk)
		free(fixed);

	return clk;

}

