#ifdef CONFIG_ARCH_SUN8IW10P1

#ifndef __SUNXI_MMC_SUN50IW1P1_3_H__
#define __SUNXI_MMC_SUN50IW1P1_3_H__

#define SUNXI_SDMMC3

#define SUNXI_DMA_TL_SDMMC3		((0x3<<28)|(15<<16)|240)
//one dma des can transfer data size = 1<<SUNXI_DES_SIZE_SDMMC2
#define SUNXI_DES_SIZE_SDMMC3	(12)

extern int sunxi_mmc_clk_set_rate_for_sdmmc3(struct sunxi_mmc_host *host,
				  struct mmc_ios *ios);
extern void sunxi_mmc_thld_ctl_for_sdmmc3(struct sunxi_mmc_host *host,
			  struct mmc_ios *ios, struct mmc_data *data);

void sunxi_mmc_save_spec_reg3(struct sunxi_mmc_host *host);
void sunxi_mmc_restore_spec_reg3(struct sunxi_mmc_host *host);
void sunxi_mmc_dump_dly3(struct sunxi_mmc_host *host);
void sunxi_mmc_do_shutdown3(struct platform_device * pdev);

#endif

#endif
