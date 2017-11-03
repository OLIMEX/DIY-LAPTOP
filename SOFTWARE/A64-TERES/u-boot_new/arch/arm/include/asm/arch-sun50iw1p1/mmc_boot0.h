#ifndef _SUNXI_MMC_BOOT0_H
#define _SUNXI_MMC_BOOT0_H

void set_mmc_para(int smc_no,void *sdly_addr );
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
int sunxi_mmc_init(int sdc_no, unsigned bus_width, const normal_gpio_cfg *gpio_info, int offset ,void *extra_data);
int sunxi_mmc_exit(int sdc_no, const normal_gpio_cfg *gpio_info, int offset);

#endif /* _SUNXI_MMC_BOOT0_H */
