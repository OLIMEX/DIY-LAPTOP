/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <sunxi_mbr.h>
#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <mmc.h>
#include <power/sunxi/axp.h>
#include <asm/io.h>
#include <power/sunxi/pmu.h>

DECLARE_GLOBAL_DATA_PTR;


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
/* add board specific code here */
int board_init(void)
{
//	gd->bd->bi_arch_number = LINUX_MACHINE_ID;
//	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100);
//	debug("board_init storage_type = %d\n",uboot_spare_head.boot_data.storage_type);
	u32 reg_val;
	//set sram for vedio use, default is boot use
	reg_val = readl(0x01c00004);
	reg_val &= ~(0x1<<24);
	writel(reg_val, 0x01c00004);

	//VE gating :brom set this bit, but not require now
	reg_val = readl(0x01c20064);
	reg_val &= ~(0x1<<0);
	writel(reg_val, 0x01c20064);

	//VE Bus Reset: brom set this bit, but not require now
	reg_val = readl(0x1c202c4);
	reg_val &= ~(0x1<<0);
	writel(reg_val, 0x1c202c4);

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
void dram_init_banksize(void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = gd->ram_size;
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
int dram_init(void)
{
	uint *addr = NULL; 
	addr = (uint *)uboot_spare_head.boot_data.dram_para;

	if(addr[4])
	{
		gd->ram_size = (addr[4] & 0xffff) * 1024 * 1024;
	}
	else
	{
		gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	}
	//reserve trusted dram
	gd->ram_size -= PLAT_TRUSTED_DRAM_SIZE;
	
	print_size(gd->ram_size, "");
	putc('\n');
	#if 0
	puts("dram_para_set start\n");
	script_parser_patch("dram_para", "dram_clk", &addr[0], 1);
	script_parser_patch("dram_para", "dram_type", &addr[1], 1);
	script_parser_patch("dram_para", "dram_zq", &addr[2], 1);
	script_parser_patch("dram_para", "dram_odt_en", &addr[3], 1);
	
	script_parser_patch("dram_para", "dram_para1", &addr[4], 1);
	script_parser_patch("dram_para", "dram_para2", &addr[5], 1);
	
	script_parser_patch("dram_para", "dram_mr0", &addr[6], 1);
	script_parser_patch("dram_para", "dram_mr1", &addr[7], 1);
	script_parser_patch("dram_para", "dram_mr2", &addr[8], 1);
	script_parser_patch("dram_para", "dram_mr3", &addr[9], 1);
	
	script_parser_patch("dram_para", "dram_tpr0", &addr[10], 1);
	script_parser_patch("dram_para", "dram_tpr1", &addr[11], 1);
	script_parser_patch("dram_para", "dram_tpr2", &addr[12], 1);
	script_parser_patch("dram_para", "dram_tpr3", &addr[13], 1);
	script_parser_patch("dram_para", "dram_tpr4", &addr[14], 1);
	script_parser_patch("dram_para", "dram_tpr5", &addr[15], 1);
	script_parser_patch("dram_para", "dram_tpr6", &addr[16], 1);
	script_parser_patch("dram_para", "dram_tpr7", &addr[17], 1);
	script_parser_patch("dram_para", "dram_tpr8", &addr[18], 1);
	script_parser_patch("dram_para", "dram_tpr9", &addr[19], 1);
	script_parser_patch("dram_para", "dram_tpr10", &addr[20], 1);
	script_parser_patch("dram_para", "dram_tpr11", &addr[21], 1);
	script_parser_patch("dram_para", "dram_tpr12", &addr[22], 1);
	script_parser_patch("dram_para", "dram_tpr13", &addr[23], 1);
	puts("dram_para_set end\n");
	#endif
	return 0;
}

#ifdef CONFIG_GENERIC_MMC

extern int sunxi_mmc_init(int sdc_no);

int board_mmc_init(bd_t *bis)
{
	sunxi_mmc_init(bis->bi_card_num);

	return 0;
}

void board_mmc_pre_init(int card_num)
{
	bd_t *bd;

	bd = gd->bd;
	gd->bd->bi_card_num = card_num;
	mmc_initialize(bd);
  
}

int board_mmc_get_num(void)
{
    return gd->boot_card_num;
}


void board_mmc_set_num(int num)
{
    gd->boot_card_num = num;
}

#endif



#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board: SUN6I\n");
	return 0;
}
#endif

int cpu0_set_detected_paras(void)
{
	return 0;
}

ulong get_spare_head_size(void)
{
	return (ulong)sizeof(struct spare_boot_head_t);
}

extern int axp81_probe(void);

/**
 * platform_axp_probe -detect the pmu on  board
 * @sunxi_axp_dev_pt: pointer to the axp array
 * @max_dev: offset of the property to retrieve
 * returns:
 *	the num of pmu
 */

int platform_axp_probe(sunxi_axp_dev_t  *sunxi_axp_dev_pt[], int max_dev)
{
	if(axp81_probe())
	{
		printf("probe axp81X failed\n");
		sunxi_axp_dev_pt[0] = &sunxi_axp_null;
		return 0;
	}
	
	/* pmu type AXP81X */
	tick_printf("PMU: AXP81X found\n");

	sunxi_axp_dev_pt[0] = &sunxi_axp_81;
	sunxi_axp_dev_pt[PMU_TYPE_81X] = &sunxi_axp_81;
	//find one axp
	return 1;

}

char* board_hardware_info(void)
{
	static char * hardware_info  = "sun50iw1p1";
	return hardware_info;
}


