#ifndef __SUNXI_I2S_H__
#define __SUNXI_I2S_H__
#include "sunxi_dma.h"
struct sunxi_i2s {
	struct clk *pllclk;
	struct clk *moduleclk;
	void __iomem  *sunxi_i2s_membase;
	struct sunxi_dma_params play_dma_param;
	struct sunxi_dma_params capture_dma_param;
	struct snd_soc_dai_driver dai;
};
#endif