#ifndef  __USB_EFEX_TEST_H__
#define  __USB_EFEX_TEST_H__

#include <bmp_layout.h>
#include <common.h>
#include <sys_config.h>
#include <pmu.h>
#include "../fs/aw_fs/ff.h"
//下面为电压获取数据结构，
#define  BOOT2_MAX_VOLT_NUM (30) 
#pragma  pack(push,1)
typedef struct tag_PMU_VOLT_VALUE
{
	char pmu_type[16];
	char vol_name[16];
	u32 voltage;
	u32 gate;
	u8 res[24];
}PMU_VOLT_VALUE_t;
 
//大小为2K
typedef struct tag_PMUVOLT
{
	u32 voltCnt; //传回的有效电压个数
	PMU_VOLT_VALUE_t volt[BOOT2_MAX_VOLT_NUM];
	u8 res[126];
}PMU_VOLT_t;
#pragma pack(pop)

typedef struct boot_clock_t
{
	u32 cpu_id ;
	u32 freqency;
	u32 gate;
}boot_clock;

typedef struct boot_logo_info_t
{
    u32 check_sum;
    u32 boot_logo_size;
    u32 boot_logo_addr;
}boot_logo_info;

#define  FEX_CMD_BOOT2_DOWNLOAD_SYS_CONFIG 	(0x7f20)  	//下载fex配置文件，
#define  FEX_CMD_BOOT2_GET_VOLTAGE 			(0x9001)	//获取电压信息
#define  FEX_CMD_BOOT2_GET_FREQ 			(0x9002)	//获取频率信息
#define  FEX_CMD_BOOT2_GET_CURRENT_LIMIT 	(0x9004)	//获取限流寄存器信息
#define  FEX_CMD_BOOT2_DOWNLOAD_LOGO 		(0x7f21)	//下载logo
#define  FEX_CMD_BOOT2_GET_LOGO_INFO 		(0x9008) 	//获取logo
//#define  FEX_CMD_BOOT2_GET_FASTBOOT_SYSTEM_PARTITION (0x9006)
//#define  FEX_CMD_BOOT2_GET_MBR               (0x9007)

#define BOOT_LOGO_MAX_SIZE 1024*1024

extern uint replace_boot_logo(bmp_image_t *bmp_info);
extern int sunxi_probe_axp_each_volt(PMU_VOLT_t *pmu_para);
extern u32 read_file(char *file_name,uchar * buffer);


#endif
