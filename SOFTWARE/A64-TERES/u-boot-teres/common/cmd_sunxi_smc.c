/*
 * (C) Copyright 2007-2013
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <smc.h>

int do_sunxi_smc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong x0=0,x1=0,x2=0,x3=0;
	ulong *smc_result_arr = NULL;
	printf("argc is %d\n", argc);
	if(argc >= 2)
	{
		x0 = simple_strtol(argv[1], NULL, 16);
		if(argc >= 3) x1 = simple_strtol(argv[2], NULL, 16);
		if(argc >= 4) x2 = simple_strtol(argv[3], NULL, 16);
	}
	else
	{
		printf("smc params is error\n");
		return -1;
	}
	printf("smc call start:fid = %lx \n",x0);
	
	if(x0 == 0x84000003) //psci --cpu_on
	{
		//for test, x0:cmd x1: target cpu  x2:entry point
		*(volatile ulong*)(0x10000) = 0xeafffffe;// eq asm ("b .")
		x2 = 0x10000;
	}
	
	sunxi_smc_call(x0,x1,x2,x3,(ulong)&smc_result_arr);

	printf("smc call end\n");
	//smc_result_arr = (ulong*)0x4a000000;

	printf("x0 = %lx\n",smc_result_arr[0]);
	printf("x1 = %lx\n",smc_result_arr[1]);
	printf("x2 = %lx\n",smc_result_arr[2]);
	printf("x3 = %lx\n",smc_result_arr[3]);

	#if 0
	{
		u32 major=0,minor=0;
		u32 uuid[4] = {0};
		int smc_ret = 0;
		smc_ret = arm_svc_call_count();
		printf("arm svc call conut is %d\n", smc_ret);

		smc_ret = arm_svc_version(&major,&minor);
		printf("arm svc version:%d.%d\n",major,minor );

		smc_ret = arm_svc_uuid(uuid);
		printf("arm svc uuid:%08x.%08x.%08x.%08x\n",uuid[0],uuid[1],uuid[2],uuid[3] );
	}
	#endif
	return 0;
}


U_BOOT_CMD(
	sunxi_smc, 5, 0, do_sunxi_smc,
	"do a smc test",
	"sunxi_smc fid [para1,para2]"
);

