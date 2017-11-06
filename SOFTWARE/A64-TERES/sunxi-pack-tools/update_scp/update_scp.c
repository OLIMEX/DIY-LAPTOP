#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "include/arisc.h"

#define  MAX_PATH             (260)
#define HEADER_OFFSET     (0x4000)
#define ARISC_INF       //printf

static void usage(void)
{
	printf(" update_scp scp.bin dtb.bin\n");
	printf(" update arisc para from dtb\n");

	return ;
}

int IsFullName(const char *FilePath)
{
    if (isalpha(FilePath[0]) && ':' == FilePath[1])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void GetFullPath(char *dName, const char *sName)
{
    char Buffer[256];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, 256 ) == NULL)
   {
        perror( "getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

char *probe_file_data(char *file_name, int *file_len)
{
	FILE *pfile;
	int   len;
	char *buffer;

	pfile = fopen(file_name, "rb");
	if (pfile == NULL) {
		printf("file %s cant be opened\n",file_name);

		return NULL;
	}
	fseek(pfile, 0, SEEK_END);
	len = ftell(pfile);

	buffer = malloc(len);
	if (buffer == NULL) {
		printf("buffer cant be malloc\n");
		fclose(pfile);

		return NULL;
	}

	memset(buffer, 0, len);

	fseek(pfile, 0, SEEK_SET);
	fread(buffer, len, 1, pfile);
	fclose(pfile);

	*file_len = len;

	return buffer;
}

int parse_arisc_space_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;
	uint value[4] = {0, 0, 0};

	nodeoffset = fdt_path_offset(working_fdt,"/soc/arisc_space");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "space4", value);
	arisc_para->message_pool_phys = value[0];
	arisc_para->message_pool_size = value[2];

	return 0;
}


int parse_standby_space_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;
	uint value[4] = {0, 0, 0};

	nodeoffset = fdt_path_offset(working_fdt,"/soc/standby_space");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "space1", value);
	arisc_para->standby_base = value[0];
	arisc_para->standby_size = value[2];

	return 0;
}

int parse_s_uart_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/soc/s_uart");
	if(nodeoffset < 0)
	{
		return -1;
	}

	arisc_para->suart_status = fdtdec_get_is_enabled(working_fdt, nodeoffset);

	return 0;
}

int parse_pmu0_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/soc/pmu0");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "pmu_bat_shutdown_ltf", &arisc_para->pmu_bat_shutdown_ltf);
	fdt_getprop_u32(working_fdt, nodeoffset, "pmu_bat_shutdown_htf", &arisc_para->pmu_bat_shutdown_htf);
	fdt_getprop_u32(working_fdt, nodeoffset, "pmu_pwroff_vol", &arisc_para->pmu_pwroff_vol);
	fdt_getprop_u32(working_fdt, nodeoffset, "power_start", &arisc_para->power_start);

	return 0;
}

int parse_arisc_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/soc/arisc");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "powchk_used", &arisc_para->powchk_used);
	fdt_getprop_u32(working_fdt, nodeoffset, "power_reg", &arisc_para->power_reg );
	fdt_getprop_u32(working_fdt, nodeoffset, "system_power", &arisc_para->system_power);

	return 0;
}

int parse_s_cir_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;
	int i;
	char subkey[64];

	nodeoffset = fdt_path_offset(working_fdt, "/soc/s_cir");
	if(nodeoffset < 0)
	{
		return -1;
	}

	if (fdt_getprop_u32(working_fdt, nodeoffset, "ir_power_key_code", &arisc_para->ir_key.ir_code_depot[0].key_code)>0)
	{
		if (fdt_getprop_u32(working_fdt, nodeoffset, "ir_addr_code", &arisc_para->ir_key.ir_code_depot[0].addr_code)>0)
		{
			arisc_para->ir_key.num = 1;
			goto print_ir_paras;
		}
	}

	arisc_para->ir_key.num = 0;
	for (i = 0; i < IR_NUM_KEY_SUP; i++) {
	sprintf(subkey, "%s%d", "ir_power_key_code", i);
	if (fdt_getprop_u32(working_fdt, nodeoffset, subkey, &arisc_para->ir_key.ir_code_depot[i].key_code)>0) {
		sprintf(subkey, "%s%d", "ir_addr_code", i);
		if (fdt_getprop_u32(working_fdt, nodeoffset, subkey, &arisc_para->ir_key.ir_code_depot[i].addr_code)>0) {
			arisc_para->ir_key.num++;
		}
	}
}

print_ir_paras:
	for (i = 0; i < arisc_para->ir_key.num; i++)
	{
		ARISC_INF(" ir_code[%u].key_code:0x%x, ir_code[%u].addr_code:0x%x\n",
			i, arisc_para->ir_key.ir_code_depot[i].key_code, i,  arisc_para->ir_key.ir_code_depot[i].addr_code);
	}

	return 0;
}

int parse_dram_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/dram");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "dram_clk",   		&arisc_para->dram_para.dram_clk);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_type",  	&arisc_para->dram_para.dram_type);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_zq",   		&arisc_para->dram_para.dram_zq);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_odt_en", 	&arisc_para->dram_para.dram_odt_en);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_para1",	&arisc_para->dram_para.dram_para1);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_para2",	&arisc_para->dram_para.dram_para2);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_mr0",  	&arisc_para->dram_para.dram_mr0);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_mr1",  	&arisc_para->dram_para.dram_mr1);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_mr2",  	&arisc_para->dram_para.dram_mr2);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_mr3",  	&arisc_para->dram_para.dram_mr3);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr0",    	&arisc_para->dram_para.dram_tpr0);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr1",    	&arisc_para->dram_para.dram_tpr1);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr2",    	&arisc_para->dram_para.dram_tpr2);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr3",    	&arisc_para->dram_para.dram_tpr3);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr4",    	&arisc_para->dram_para.dram_tpr4);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr5",   	&arisc_para->dram_para.dram_tpr5);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr6",    	&arisc_para->dram_para.dram_tpr6);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr7",    	&arisc_para->dram_para.dram_tpr7);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr8",    	&arisc_para->dram_para.dram_tpr8);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr9",    	&arisc_para->dram_para.dram_tpr9);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr10",   	&arisc_para->dram_para.dram_tpr10);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr11",   	&arisc_para->dram_para.dram_tpr11);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr12",   	&arisc_para->dram_para.dram_tpr12);
	fdt_getprop_u32(working_fdt, nodeoffset,  "dram_tpr13",   	&arisc_para->dram_para.dram_tpr13);

	return 0;
}

int parse_dvfs_table_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int 	nodeoffset;
	int 	i;
	uint val;
	uint vf_table_count = 0;
	uint vf_table_type = 0;
	char vf_table_main_key[64];
	char vf_table_sub_key[64];
	uint  vf_table_size = 0;

	nodeoffset = fdt_path_offset(working_fdt, "/dvfs_table");
	if (nodeoffset < 0)
	{
		return -1;
	}

	if (fdt_getprop_u32(working_fdt, nodeoffset, "vf_table_count", &vf_table_count)<0)
	{
		ARISC_INF("%s: support only one vf_table\n", __func__);
		sprintf(vf_table_main_key, "%s", "/dvfs_table");
	}
	else
	{
		sprintf(vf_table_main_key, "%s%d", "/vf_table", vf_table_type);
	}
	ARISC_INF("%s: vf table type [%d=%s]\n", __func__, vf_table_type, vf_table_main_key);

	nodeoffset = fdt_path_offset(working_fdt, vf_table_main_key);

	/* parse system config v-f table information */
	fdt_getprop_u32(working_fdt, nodeoffset, "lv_count", &vf_table_size);

	for (i = 0; i < vf_table_size; i++)
	{
		sprintf(vf_table_sub_key, "lv%d_freq", i + 1);
		if (fdt_getprop_u32(working_fdt, nodeoffset, vf_table_sub_key, &val)>0)
		{
			arisc_para->vf[i].freq = val;
		}
		ARISC_INF("%s: freq [%s-%d=%d]\n", __func__, vf_table_sub_key, i, val);
		sprintf(vf_table_sub_key, "lv%d_volt", i + 1);
		if (fdt_getprop_u32(working_fdt, nodeoffset, vf_table_sub_key, &val))
		{
			arisc_para->vf[i].voltage = val;
		}
		ARISC_INF("%s: volt [%s-%d=%d]\n", __func__, vf_table_sub_key, i, val);
	}

	return 0;
}

int parse_box_start_os0_node(const void*working_fdt,arisc_para_t *arisc_para)
{
	int 	nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/soc/box_start_os0");
	if(nodeoffset < 0)
	{
		return -1;
	}

	arisc_para->start_os.used = fdtdec_get_is_enabled(working_fdt, nodeoffset);
	fdt_getprop_u32(working_fdt, nodeoffset, "start_type", &arisc_para->start_os.start_type);
	fdt_getprop_u32(working_fdt, nodeoffset, "irkey_used", &arisc_para->start_os.irkey_used);
	fdt_getprop_u32(working_fdt, nodeoffset, "pmukey_used", &arisc_para->start_os.pmukey_used);

	return 0;
}

int parse_power_mode(const void*working_fdt,arisc_para_t *arisc_para)
{
	int 	nodeoffset;

	nodeoffset = fdt_path_offset(working_fdt, "/soc/target");
	if(nodeoffset < 0)
	{
		return -1;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "power_mode", &arisc_para->power_mode);
}

int main(int argc, char *argv[])
{
	char dtbpath[MAX_PATH]="";
	char scppath[MAX_PATH]="";
	FILE *scp_file;
	char *working_fdt;
	char *working_scp;
	int  dtb_len,scp_len,i;
	arisc_para_t *arisc_para;

	if(argc != 3)
	{
		printf("parameters invalid\n");
		usage();
		return -1;
	}

	GetFullPath(dtbpath, argv[2]);
	GetFullPath(scppath, argv[1]);
	printf("dtbpath=%s\n", dtbpath);
	printf("scppath=%s\n", scppath);

	working_fdt = probe_file_data(dtbpath, &dtb_len);
	working_scp = probe_file_data(scppath, &scp_len);

	if ((working_fdt == NULL) ||(working_scp == NULL))
	{
		printf("file invalid\n");
		return -1;
	}

	scp_file = fopen(scppath, "wb");
	if (scp_file == NULL)
	{
		printf("file %s cant be opened\n",scppath);
		return -1;
	}

	arisc_para = (arisc_para_t *)(working_scp+HEADER_OFFSET);

	parse_arisc_space_node(working_fdt,arisc_para);
	parse_standby_space_node(working_fdt,arisc_para);
	parse_s_uart_node(working_fdt,arisc_para);
	parse_pmu0_node(working_fdt,arisc_para);
	parse_arisc_node(working_fdt,arisc_para);
	parse_s_cir_node(working_fdt,arisc_para);
	parse_dram_node(working_fdt,arisc_para);
	parse_dvfs_table_node(working_fdt,arisc_para);
	parse_box_start_os0_node(working_fdt,arisc_para);
	parse_power_mode(working_fdt,arisc_para);

	fwrite(working_scp, scp_len, 1, scp_file);
	fclose(scp_file);

	if (working_fdt)
		free(working_fdt);
	if (working_scp)
		free(working_scp);

	printf("update scp finish!\n");

	return 0;
}

