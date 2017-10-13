#ifndef __SUNXI_MMC_EXPORT_H__
#define __SUNXI_MMC_EXPORT_H__
void sunxi_mmc_rescan_card(unsigned id);
void sunxi_mmc_reg_ex_res_inter(struct sunxi_mmc_host *host,u32 phy_id);
int sunxi_mmc_check_r1_ready(struct mmc_host* mmc, unsigned ms);


#endif
