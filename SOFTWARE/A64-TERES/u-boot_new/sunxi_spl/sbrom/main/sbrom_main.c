/*
************************************************************************************************************************
*                                          Boot rom
*                                         Seucre Boot
*
*                             Copyright(C), 2006-2013, AllWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name   : Base.h
*
* Author      : glhuang
*
* Version     : 0.0.1
*
* Date        : 2013.09.05
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* glhuang       2013.09.05       0.0.1        build the file
*
************************************************************************************************************************
*/
#include "common.h"
#include "sbrom_toc.h"
#include "boot_type.h"
#include "openssl_ext.h"
#include "asm/arch/clock.h"
#include "asm/arch/ss.h"
#include "asm/arch/timer.h"
#include "asm/arch/uart.h"
#include "asm/arch/rtc_region.h"
#include "asm/arch/mmu.h"
#include "asm/arch/gic.h"
#include "private_toc.h"
#include "sbrom_toc.h"
#include "../libs/sbrom_libs.h"
#include <asm/arch/dram.h>
#include <private_toc.h>


extern void sid_read_rotpk(void *dst);
extern void sunxi_certif_mem_reset(void);
//extern int sunxi_certif_probe_pubkey(X509 *x, sunxi_key_t *pubkey);
extern void ndump(u8 *buf, int count);

static void print_commit_log(void);
static int sbromsw_toc1_traverse(void);
//static int sbromsw_probe_fel_flag(void);
static int sbromsw_clear_env(void);
//static int sunxi_root_certif_pk_verify(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len);
#ifdef SUNXI_OTA_TEST
static int sbromsw_print_ota_test(void);
#endif
sbrom_toc0_config_t *toc0_config = (sbrom_toc0_config_t *)CONFIG_TOC0_CONFIG_ADDR;
extern char sbromsw_hash_value[64];

void sbromsw_entry(void)
{
	toc0_private_head_t *toc0 = (toc0_private_head_t *)CONFIG_SBROMSW_BASE;
	uint dram_size;
	int  ret, flag;

	timer_init();
	sunxi_serial_init(toc0_config->uart_port, toc0_config->uart_ctrl, 2);
    set_debugmode_flag();
    print_commit_log();

	{
		volatile int a=12;
		while(a==1);
	}

    set_pll();
	printf("try to probe rtc region\n");
	flag = rtc_region_probe_fel_flag();
	printf("flag=0x%x\n", flag);
	if(flag == SUNXI_RUN_EFEX_FLAG)
	{
		printf("sbromsw_entry sbromsw_probe_fel_flag\n");
		rtc_region_clear_fel_flag();

		goto __sbromsw_entry_err0;
	}
	printf("try to setup mmu\n");
	//mmu init
	mmu_setup();
	printf("mmu setup ok\n");
	//dram init
	printf("try to init dram\n");
	dram_size = init_DRAM(0, (void *)toc0_config->dram_para);
	if (dram_size)
	{
		printf("init dram ok, size=%dM\n", dram_size);
	}
	else
	{
		printf("init dram fail\n");

		goto __sbromsw_entry_err;
	}
	printf("mmu resetup\n");
	//mmu_resetup(dram_size, toc0_config->secure_dram_mbytes);
	printf("init heap\n");
	create_heap(CONFIG_HEAP_BASE, CONFIG_HEAP_SIZE);
	printf("init gic\n");
//	gic_init();
	printf("init flash\n");
	ret = sunxi_flash_init(toc0->platform[0] & 0x0f);		//初始化外部介质，准备读取TOC1数据
	if(ret)
	{
		printf("sbromsw_entry sunxi_flash_init failed\n");

		goto __sbromsw_entry_err;
	}
	ret = toc1_init();      //TOC1初始化，判断TOC1的头部是否合格
	if(ret)
	{
		printf("sbromsw_entry toc1_init failed\n");

		goto __sbromsw_entry_err;
	}
	ret = sbromsw_toc1_traverse();
	if(ret)
	{
		printf("sbromsw_entry sbromsw_toc1_traverse failed\n");

		goto __sbromsw_entry_err;
	}

__sbromsw_entry_err:
__sbromsw_entry_err0:
	sbromsw_clear_env();

	boot0_jump(SUNXI_FEL_ADDR_IN_SECURE);
}

static void print_commit_log(void)
{
        printf("sbrom commit : %s \n",sbromsw_hash_value);
        return ;
}

#define  SUNXI_X509_CERTIFF_MAX_LEN   (4096)

static int sbromsw_toc1_traverse(void)
{
	sbrom_toc1_item_group item_group;
	int ret;
	uint len, i;
	u8 buffer[SUNXI_X509_CERTIFF_MAX_LEN];

	sunxi_certif_info_t  root_certif;
	sunxi_certif_info_t  sub_certif;
	u8  hash_of_file[256];
	//u8  hash_in_certif[256];

	//u8  key_certif_extension[260];
	//u8  content_certif_key[520];
	//int out_to_ns;
    //int ready_out_to_ns = 0;
    //uint optee_entry=0, uboot_entry=0;
    uint monitor_entry=0;

	toc1_item_traverse();

	printf("probe root certif\n");
	sunxi_ss_open();

	memset(buffer, 0, SUNXI_X509_CERTIFF_MAX_LEN);
	len = toc1_item_read_rootcertif(buffer, SUNXI_X509_CERTIFF_MAX_LEN);
	if(!len)
	{
		printf("%s error: cant read rootkey certif\n", __func__);

		return -1;
	}
	{
		volatile int a=2;
		while(a==1);
	}
	if(sunxi_certif_verify_itself(&root_certif, buffer, len))
	{
		printf("certif invalid: root certif verify itself failed\n");

		return -1;
	}
	do
	{
		memset(&item_group, 0, sizeof(sbrom_toc1_item_group));
		ret = toc1_item_probe_next(&item_group);
		if(ret < 0)
		{
			printf("sbromsw_toc1_traverse err in toc1_item_probe_next\n");

			return -1;
		}
		else if(ret == 0)
		{
			printf("sbromsw_toc1_traverse find out all items\n");

			printf("monitor entry=0x%x\n", monitor_entry);
			//printf("uboot entry=0x%x\n", uboot_entry);

			//if(optee_entry && uboot_entry)
				//go_exec(optee_entry, uboot_entry, SECURE_SWITCH_NORMAL);
			if(monitor_entry)
				go_exec(monitor_entry, 0, 0);

			return 0;
		}
		if(item_group.bin_certif)
		{
			memset(buffer, 0, SUNXI_X509_CERTIFF_MAX_LEN);
			len = toc1_item_read(item_group.bin_certif, buffer, SUNXI_X509_CERTIFF_MAX_LEN);
			if(!len)
			{
				printf("%s error: cant read content key certif\n", __func__);

				return -1;
			}
			//证书内容进行自校验，确保没有被替换
			if(sunxi_certif_verify_itself(&sub_certif, buffer, len))
			{
				printf("%s error: cant verify the content certif\n", __func__);

				return -1;
			}
//			printf("key n:\n");
//			ndump(sub_certif.pubkey.n, sub_certif.pubkey.n_len);
//			printf("key e:\n");
//			ndump(sub_certif.pubkey.e, sub_certif.pubkey.e_len);
			//每当发现一个公钥证书，即在根证书中寻找匹配项目，找不到则认为有错误
			for(i=0;i<root_certif.extension.extension_num;i++)
			{
				if(!strcmp((const char *)root_certif.extension.name[i], item_group.bin_certif->name))
				{
					printf("find %s key stored in root certif\n", item_group.bin_certif->name);

					if(memcmp(root_certif.extension.value[i], sub_certif.pubkey.n+1, sub_certif.pubkey.n_len-1))
					{
						printf("%s key n is incompatible\n", item_group.bin_certif->name);
						printf(">>>>>>>key in rootcertif<<<<<<<<<<\n");
						ndump((u8 *)root_certif.extension.value[i], sub_certif.pubkey.n_len-1);
						printf(">>>>>>>key in certif<<<<<<<<<<\n");
						ndump((u8 *)sub_certif.pubkey.n+1, sub_certif.pubkey.n_len-1);

						return -1;
					}
					if(memcmp(root_certif.extension.value[i] + sub_certif.pubkey.n_len-1, sub_certif.pubkey.e, sub_certif.pubkey.e_len))
					{
						printf("%s key e is incompatible\n", item_group.bin_certif->name);
						printf(">>>>>>>key in rootcertif<<<<<<<<<<\n");
						ndump((u8 *)root_certif.extension.value[i] + sub_certif.pubkey.n_len-1, sub_certif.pubkey.e_len);
						printf(">>>>>>>key in certif<<<<<<<<<<\n");
						ndump((u8 *)sub_certif.pubkey.e, sub_certif.pubkey.e_len);

						return -1;
					}
					break;
				}
			}
			if(i==root_certif.extension.extension_num)
			{
				printf("cant find %s key stored in root certif", item_group.bin_certif->name);

				return -1;
			}
		}

		if(item_group.binfile)
		{
			//读出bin文件内容到内存
			len = sunxi_flash_read(item_group.binfile->data_offset/512, (item_group.binfile->data_len+511)/512, (void *)item_group.binfile->run_addr);
			//len = sunxi_flash_read(item_group.binfile->data_offset/512, (item_group.binfile->data_len+511)/512, (void *)0x2a000000);
			if(!len)
			{
				printf("%s error: cant read bin file\n", __func__);

				return -1;
			}
			//计算文件hash
			memset(hash_of_file, 0, sizeof(hash_of_file));
			ret = sunxi_sha_calc(hash_of_file, sizeof(hash_of_file), (u8 *)item_group.binfile->run_addr, item_group.binfile->data_len);
			//ret = sunxi_sha_calc(hash_of_file, sizeof(hash_of_file), (u8 *)0x2a000000, item_group.binfile->data_len);
			if(ret)
			{
				printf("sunxi_sha_calc: calc sha256 with hardware err\n");

				return -1;
			}
			//使用内容证书的扩展项，和文件hash进行比较
			//开始比较文件hash(小机端阶段计算得到)和证书hash(PC端计算得到)
			if(memcmp(hash_of_file, sub_certif.extension.value[0], 32))
			{
				printf("hash compare is not correct\n");
				printf(">>>>>>>hash of file<<<<<<<<<<\n");
				ndump((u8 *)hash_of_file, 32);
				printf(">>>>>>>hash in certif<<<<<<<<<<\n");
				ndump((u8 *)sub_certif.extension.value[0], 32);

				return -1;
			}

			printf("ready to run %s\n", item_group.binfile->name);
            if(!toc0_config->secure_without_OS)
            {
                ;//ready_out_to_ns = 1;
            }
            else
            {
                printf("secure_without_OS mode  \n");
                ;//ready_out_to_ns = 0;
            }

            if(!strcmp(item_group.binfile->name, "monitor"))
            {
            	monitor_entry = item_group.binfile->run_addr;
            }
			//else if(strcmp(item_group.binfile->name, "u-boot"))
			//{
			//	;//out_to_ns = SECURE_SWITCH_OTHER;
			//}
			else
			{
//                if(!ready_out_to_ns)
//					out_to_ns = SECURE_NON_SECUREOS;
//                else
//                    out_to_ns = SECURE_SWITCH_NORMAL;
//                uboot_entry = item_group.binfile->run_addr;
			}
			//toc0_config->next_exe_pa   = va2pa(item_group.binfile->run_addr);
			//go_exec(item_group.binfile->run_addr, CONFIG_TOC0_CONFIG_ADDR, out_to_ns);
		}
	}
	while(1);

	return 0;
}

//static int sbromsw_probe_fel_flag(void)
//{
//	uint flag;
//
//	flag = rtc_region_probe_fel_flag();
//	rtc_region_clear_fel_flag();
//
//	return flag;
//}

#ifdef SUNXI_OTA_TEST
static int sbromsw_print_ota_test(void)
{
	printf("*********************************************\n");
	printf("*********************************************\n");
	printf("*********************************************\n");
	printf("*********************************************\n");
	printf("********[OTA TEST]:update toc0 sucess********\n");
	printf("*********************************************\n");
	printf("*********************************************\n");
	printf("*********************************************\n");
	printf("*********************************************\n");
	return 0;
}
#endif

static int sbromsw_clear_env(void)
{
//	gic_exit();
//	reset_pll();
//	mmu_turn_off();

	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
#define RSA_BIT_WITDH 2048
//static int sunxi_certif_pubkey_check( sunxi_key_t  *pubkey )
//{
//	char efuse_hash[256] , rotpk_hash[256];
//	char all_zero[32];
//
//	char pk[RSA_BIT_WITDH/8 * 2 + 256]; /*For the stupid sha padding */
//
//	sid_read_rotpk(efuse_hash);
//	memset(all_zero, 0, 32);
//	if( ! memcmp(all_zero, efuse_hash,32 ) )
//		return 0 ; /*Don't check if rotpk efuse is empty*/
//	else{
//		memset(pk, 0x91, sizeof(pk));
//		char *align = (char *)(((u32)pk+31)&(~31));
//		if( *(pubkey->n) ){
//			memcpy(align, pubkey->n, pubkey->n_len);
//			memcpy(align+pubkey->n_len, pubkey->e, pubkey->e_len);
//		}else{
//			memcpy(align, pubkey->n+1, pubkey->n_len-1);
//			memcpy(align+pubkey->n_len-1, pubkey->e, pubkey->e_len);
//		}
//
//		if(sunxi_sha_calc( (u8 *)rotpk_hash, 32, (u8 *)align, RSA_BIT_WITDH/8*2 ))
//		{
//			printf("sunxi_sha_calc: calc  pubkey sha256 with hardware err\n");
//			return -1;
//		}
//
//		if(memcmp(rotpk_hash, efuse_hash, 32)){
//			printf("certif pk dump\n");
//			ndump((u8 *)align , RSA_BIT_WITDH/8*2 );
//
//			printf("calc certif pk hash dump\n");
//			ndump((u8 *)rotpk_hash,32);
//
//			printf("efuse pk dump\n");
//			ndump((u8 *)efuse_hash,32);
//
//			printf("sunxi_certif_pubkey_check: pubkey hash check err\n");
//			return -1;
//		}
//		return 0 ;
//	}
//
//}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  buf: 证书存放起始   len：数据长度
*
*    return        :
*
*    note          :  证书自校验
*
*
************************************************************************************************************
*/
//static int sunxi_root_certif_pk_verify(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len)
//{
//	X509 *certif;
//	int  ret;
//
//	//内存初始化
//	sunxi_certif_mem_reset();
//	//创建证书
//	ret = sunxi_certif_create(&certif, buf, len);
//	if(ret < 0)
//	{
//		printf("fail to create a certif\n");
//
//		return -1;
//	}
//	//获取证书公钥
//	ret = sunxi_certif_probe_pubkey(certif, &sunxi_certif->pubkey);
//	if(ret)
//	{
//		printf("fail to probe the public key\n");
//
//		return -1;
//	}
//	ret = sunxi_certif_pubkey_check(&sunxi_certif->pubkey);
//	if(ret){
//		printf("fail to check the public key hash against efuse\n");
//
//		return -1;
//	}
//
//	sunxi_certif_free(certif);
//
//	return 0;
//}
