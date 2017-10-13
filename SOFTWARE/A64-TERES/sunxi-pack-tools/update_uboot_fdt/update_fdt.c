#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "uboot_head.h"


typedef unsigned int u32;


void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("update_fdt.exe u-boot.bin xxx.dtb output_file.bin\n\n");
}

u32 randto1k(u32 num)
{
	if(num % 1024)
	{
		printf("update_fdt: num %d randto1k\n", num);
		return ((num+1023)/1024 * 1024);
	}
	else
	{
		return num;
	}
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

int main(int argc, char* argv[])
{
	char   file_name1[256];
	char   file_name2[256];
	char   file_name3[256];
	FILE  *file1 = NULL;
	FILE  *file2 = NULL;
	FILE  *file3 = NULL;
	unsigned int file1_len, file2_len,file1_len_align,file2_len_align;
	unsigned char   *file1_buffer = NULL;
	unsigned char   *file2_buffer = NULL;
	unsigned char   *file_dst_buffer = NULL;
	//struct andr_img_hdr * img_hdr = NULL;
	struct spare_boot_head_t  * img_hdr = NULL;

	if(argc != 4 )
	{
		Usage();
		return  -1;
	}

	printf("update_fdt: %s will merage %s, generate %s\n", argv[1], argv[2], argv[3]);
	memset(file_name1, 0, sizeof(file_name1));
	memset(file_name2, 0, sizeof(file_name2));
	memset(file_name3, 0, sizeof(file_name3));

	
	/*strcpy(file_name1, "boot.img");
	strcpy(file_name2, "sun50iw1p1.dtb");
	strcpy(file_name3, "test.img");*/
	GetFullPath(file_name1, argv[1]);
	GetFullPath(file_name2, argv[2]);
	GetFullPath(file_name3, argv[3]);

	//printf("%s will concatenate %s, generate %s", file_name1, file_name2, file_name3);
	
	
	//���ļ����򲻿���ʧ��
	file1 = fopen(file_name1, "rb");
	if(!file1)
	{
		printf("unable to open  file %s\n", file_name1);
	
		goto _err_out;
	}
	
	
	//���ļ����򲻿���ʧ��
	file2 = fopen(file_name2, "rb");
	if(!file2)
	{
		printf("unable to open file %s\n", file_name2);
	
		goto _err_out;
	}
	
	//file1
	fseek(file1, 0, SEEK_END);
	file1_len = ftell(file1);
	fseek(file1, 0, SEEK_SET);
	//�������е�����
	file1_buffer = (char *)malloc(file1_len+1);
	if(file1_buffer == NULL)
	{
		printf("unable to malloc memory for udpate_fdt \n");
		goto _err_out;
	}
	memset(file1_buffer,0,file1_len+1);
	fread(file1_buffer, 1, file1_len, file1);
	fclose(file1); file1 = NULL;
	
	//img_hdr = (struct andr_img_hdr *)file1_buffer;
	img_hdr = (struct spare_boot_head_t  *)file1_buffer;
	if (memcmp(img_hdr->boot_head.magic, UBOOT_MAGIC, 5)) {
		puts("update_fdt: bad boot image magic, maybe not a uboot?\n");
		goto _err_out;
	}
	
	//file2
	fseek(file2, 0, SEEK_END);
	file2_len = ftell(file2);
	fseek(file2, 0, SEEK_SET);
	//�������е�����
	file2_buffer = (char *)malloc(file2_len+1);
	if(file2_buffer == NULL)
	{
		printf("unable to malloc memory for udpate_fdt \n");
		goto _err_out;
	}
	memset(file2_buffer,0,file2_len+1);
	fread(file2_buffer, 1, file2_len, file2);
	fclose(file2); file2=NULL;
	
	printf("---0x%x, 0x%x\n",file2_buffer[0],file2_buffer[1]);
	if(!(file2_buffer[0] == 0xD0 && file2_buffer[1] == 0x0D))
	{
		puts("update_fdt: bad dtb magic, maybe not a dtb file?\n");
		goto _err_out;
	}
	
	
	file1_len_align = randto1k(file1_len);
	file2_len_align = randto1k(file2_len);
	printf("file1_len = %x, file2_len = %x\n", file1_len_align, file2_len_align);
	
	//update head
	if(img_hdr->boot_data.dtb_offset != 0)
	{
		//update_uboot will check ,so clear it
		img_hdr->boot_head.length = 0;
		img_hdr->boot_head.uboot_length = 0;
		//update uboot len
		file1_len = file1_len_align = img_hdr->boot_data.dtb_offset;
	}
	else
	{
		img_hdr->boot_data.dtb_offset = file1_len_align;
	}
	//img_hdr->boot_data.dtb_size = file2_len_align;
	printf("dtb offset %x,size %x \n", img_hdr->boot_data.dtb_offset,file2_len_align);
	
	
	file_dst_buffer = (char *)malloc(file1_len_align+file2_len_align);
	if(file_dst_buffer == NULL)
	{
		printf("unable to malloc memory for udpate_fdt \n");
		goto _err_out;
	}
	memset(file_dst_buffer,0, file1_len_align+file2_len_align);
	memcpy(file_dst_buffer, file1_buffer, file1_len);
	memcpy(file_dst_buffer+file1_len_align, file2_buffer, file2_len);
	
	//���ļ����򲻿���ʧ��
	file3 = fopen(file_name3, "wb");
	if(!file3)
	{
		printf("unable to open file %s\n", file_name3);
		goto _err_out;
	} 
	fwrite(file_dst_buffer, file1_len_align+file2_len_align, 1, file3);
	fclose(file3); file3 = NULL;
	
	printf("update_fdt:genrate %s ok\n",file_name3);
	free(file_dst_buffer);
	free(file1_buffer);
	free(file2_buffer);
	return 0;
	
_err_out:
	if(file_dst_buffer != NULL) free(file_dst_buffer);
	if(file1_buffer != NULL) free(file1_buffer);
	if(file2_buffer != NULL) free(file2_buffer);
	
	return -1;
}
