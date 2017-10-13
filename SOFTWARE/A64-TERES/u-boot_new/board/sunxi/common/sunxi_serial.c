
#include <common.h>
#include <sys_partition.h>
#include <fdt_support.h>
#include <sunxi_board.h>
#include <asm/io.h>
#include <smc.h>

int get_serial_num_from_file(char* serial)
{
	int nodeoffset;
	int ret = 0;
	int partno = -1;
	char *filename = NULL;
	char part_info[16] = {0};  // format is "partno:0"
	char addr_info[32] = {0};  //"00000000"
	char file_info[64] = {0};
	char * bmp_argv[6] = { "fatload", "sunxi_flash", part_info, addr_info, file_info, NULL };

	//check serial_feature config info
	nodeoffset = fdt_path_offset (working_fdt, "/soc/serial_feature");
	if (nodeoffset < 0)
	{
		printf("sunxi_serial: serial_feature is not exist\n");
		return -1;
	}
	ret = fdt_getprop_string(working_fdt,nodeoffset,"sn_filename",&filename);
	if((ret < 0) || (strlen(filename)== 0) )
	{
		printf("sunxi_serial: sn_filename is not exist\n");
		return -1;
	}
	//check private partition info
	partno = sunxi_partition_get_partno_byname("private");
	if(partno < 0)
	{
		return -1;
	}

	//get data from file
	sprintf(part_info,"%d:0", partno);
	sprintf(addr_info,"%lx", (ulong)serial);
	sprintf(file_info,"%s", filename);
	if(do_fat_fsload(0, 0, 5, bmp_argv))
	{
		printf("load file(%s) error \n", bmp_argv[4]);
		return -1;
	}
	return 0;
}

int get_serial_num_from_chipid(char* serial)
{
	u32 sunxi_soc_chipid[4];
	u32 sunxi_serial[3];

	sunxi_soc_chipid[0] = arm_svc_read_sec_reg(SUNXI_SID_BASE + 0x200);
	sunxi_soc_chipid[1] = arm_svc_read_sec_reg(SUNXI_SID_BASE + 0x200 + 0x4);
	sunxi_soc_chipid[2] = arm_svc_read_sec_reg(SUNXI_SID_BASE + 0x200 + 0x8);
	sunxi_soc_chipid[3] = arm_svc_read_sec_reg(SUNXI_SID_BASE + 0x200 + 0xc);

	sunxi_serial[0] = sunxi_soc_chipid[3];
	sunxi_serial[1] = sunxi_soc_chipid[2];
	sunxi_serial[2] = (sunxi_soc_chipid[1] >> 16) & 0xFFFF;

	sprintf(serial , "%04x%08x%08x",sunxi_serial[2], sunxi_serial[1], sunxi_serial[0]);
	return 0;
}

int sunxi_set_serial_num(void)
{
	char serial[128] = {0};
	if(get_serial_num_from_file(serial))
	{
		get_serial_num_from_chipid(serial);
	}
	printf("serial is: %s\n",serial);
	if(setenv("sunxi_serial", serial))
	{
		printf("error:set variable [sunxi_serial] fail\n");
	}
	return 0;
}