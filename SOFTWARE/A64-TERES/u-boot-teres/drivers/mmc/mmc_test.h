#ifndef _MMC_TEST_H_
#define _MMC_TEST_H_

//#define MMC_INTERNAL_TEST

#ifdef MMC_INTERNAL_TEST
int mmc_t_rwc(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_erase(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_trim(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_discard(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_secure_erase(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_secure_trim(struct mmc *mmc, ulong start, ulong blkcnt);
int mmc_t_emmc_sanitize(struct mmc *mmc);
#endif /*MMC_INTERNAL_TEST*/

#endif /*_MMC_TEST_H_*/
