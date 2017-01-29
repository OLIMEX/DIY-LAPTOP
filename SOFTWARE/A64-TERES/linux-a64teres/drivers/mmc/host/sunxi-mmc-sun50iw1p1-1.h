#ifdef CONFIG_ARCH_SUN50IW1P1

#ifndef __SUNXI_MMC_SUN50IW1P1_1_H__
#define __SUNXI_MMC_SUN50IW1P1_1_H__

#define SUNXI_SDMMC1

#define SUNXI_DMA_TL_SDMMC1 ((0x3<<28)|(15<<16)|240)
//one dma des can transfer data size = 1<<SUNXI_DES_SIZE_SDMMC1
#define SUNXI_DES_SIZE_SDMMC1	(15)


extern int sunxi_mmc_clk_set_rate_for_sdmmc1(struct sunxi_mmc_host *host,
				  struct mmc_ios *ios);
extern void sunxi_mmc_thld_ctl_for_sdmmc1(struct sunxi_mmc_host *host,
			  struct mmc_ios *ios, struct mmc_data *data);

void sunxi_mmc_save_spec_reg1(struct sunxi_mmc_host *host);
void sunxi_mmc_restore_spec_reg1(struct sunxi_mmc_host *host);
#endif

#endif
