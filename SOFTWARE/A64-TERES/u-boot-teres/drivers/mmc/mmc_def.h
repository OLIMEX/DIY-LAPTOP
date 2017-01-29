#ifndef __MMC_DEF__
#define __MMC_DEF__

//#define SUNXI_MMCDBG

#ifdef SUNXI_MMCDBG
#define MMCINFO(fmt, args...)	printf("[mmc]: "fmt,##args)//err or info
#define MMCDBG(fmt, args...)	printf("[mmc]: "fmt,##args)//dbg
#define MMCPRINT(fmt,args...)	printf(fmt,##args)//data or register and so on
#else
#define MMCINFO(fmt, args...)	printf("[mmc]: "fmt,##args)//err or info
#define MMCDBG(fmt...)
#define MMCPRINT(fmt...)
#endif

#define MMC_MSG_EN	(1U)
#define MMCMSG(d, fmt, args...) do {if ((d)->msglevel & MMC_MSG_EN)  printf("[mmc]: "fmt,##args); } while(0)

#define DRIVER_VER  "2015-06-03 13:50:00"


//secure storage relate
#define MAX_SECURE_STORAGE_MAX_ITEM             32
#define SDMMC_SECURE_STORAGE_START_ADD  (6*1024*1024/512)//6M
#define SDMMC_ITEM_SIZE                                 (4*1024/512)//4K


#endif
