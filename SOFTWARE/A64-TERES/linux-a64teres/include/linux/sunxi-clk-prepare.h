#ifndef _SUNXI_LINUX_CLK_PERPARE_H
#define _SUNXI_LINUX_CLK_PERPARE_H

#ifdef CONFIG_SUNXI_CLK_PREPARE
extern int sunxi_clk_enable_prepare(struct clk *clk);
extern int sunxi_clk_disable_prepare(struct clk *clk);
#else
static int sunxi_clk_enable_prepare(struct clk *clk){return 0;};
static int sunxi_clk_disable_prepare(struct clk *clk){ return 0; };
#endif /* CONFIG_SUNXI_CLK_PREPARE */
#endif /* _SUNXI_LINUX_CLK_PERPARE_H */
