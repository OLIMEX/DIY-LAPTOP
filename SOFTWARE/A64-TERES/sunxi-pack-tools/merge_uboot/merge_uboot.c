#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "uboot_head.h"

#define ANDR_BOOT_MAGIC "ANDROID!"
#define ANDR_BOOT_MAGIC_SIZE 8
#define ANDR_BOOT_NAME_SIZE 16
#define ANDR_BOOT_ARGS_SIZE 512

typedef unsigned int u32;



void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("merge some file to then end of uboot\n");
	printf("merge_uboot.exe u-boot.bin infile outfile mode[secmonitor|secos|scp]\n\n");
}

u32 randto1k(u32 num)
{
	if(num % 1024)
	{
		printf("merge_uboot: num %d randto1k\n", num);
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
	char   file_mode[32];
	FILE  *file1 = NULL;
	FILE  *file2 = NULL;
	FILE  *file3 = NULL;
	unsigned int file1_len, file2_len,file1_len_align,file2_len_align;
	unsigned char   *file1_buffer = NULL;
	unsigned char   *file2_buffer = NULL;
	unsigned char   *file_dst_buffer = NULL;

	struct spare_boot_head_t  * img_hdr = NULL;
	if(argc != 5 )
	{
		Usage();
		return  -1;
	}

	printf("merge_uboot: %s will merge %s, generate %s, mode %s\n", argv[1], argv[2], argv[3],argv[4]);
	memset(file_name1, 0, sizeof(file_name1));
	memset(file_name2, 0, sizeof(file_name2));
	memset(file_name3, 0, sizeof(file_name3));

	
	/*strcpy(file_name1, "boot.img");
	strcpy(file_name2, "sun50iw1p1.dtb");
	strcpy(file_name3, "test.img");*/
	GetFullPath(file_name1, argv[1]);
	GetFullPath(file_name2, argv[2]);
	GetFullPath(file_name3, argv[3]);
	strcpy(file_mode,argv[4]);

	//printf("%s will concatenate %s, generate %s", file_name1, file_name2, file_name3);
	
	
	file1 = fopen(file_name1, "rb");
	if(!file1)
	{
		printf("unable to open  file %s\n", file_name1);
	
		goto _err_out;
	}
	
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
		puts("merge_uboot: bad boot image magic, maybe not a uboot?\n");
		goto _err_out;
	}
	
	//file2
	fseek(file2, 0, SEEK_END);
	file2_len = ftell(file2);
	fseek(file2, 0, SEEK_SET);
	
	file2_buffer = (char *)malloc(file2_len+1);
	if(file2_buffer == NULL)
	{
		printf("unable to malloc memory for udpate_fdt \n");
		goto _err_out;
	}
	memset(file2_buffer,0,file2_len+1);
	fread(file2_buffer, 1, file2_len, file2);
	fclose(file2); file2=NULL;
	
	
	file1_len_align = randto1k(file1_len);
	file2_len_align = randto1k(file2_len);
	printf("file1_len = %x, file2_len = %x\n", file1_len_align, file2_len_align);
	
	//update head
	if(strcmp(file_mode,"secmonitor") == 0)
	{
		img_hdr->boot_ext[0].data[0] = file1_len_align;  //offset
		img_hdr->boot_ext[0].data[1] = file2_len_align;  //size
	}
	else if(strcmp(file_mode,"secos") == 0)
	{
		img_hdr->boot_ext[1].data[0] = file1_len_align;  //offset
		img_hdr->boot_ext[1].data[1] = file2_len_align;  //size
	}
	else if(strcmp(file_mode,"scp") == 0)
	{
		img_hdr->boot_ext[2].data[0] = file1_len_align;  //offset
		img_hdr->boot_ext[2].data[1] = file2_len_align;  //size
	}
	else
	{
		Usage();
		return -1;
	}
	
	printf("file[%s] offset %x,size %x \n",file_mode, file1_len_align,file2_len_align);
	
	
	file_dst_buffer = (char *)malloc(file1_len_align+file2_len_align);
	if(file_dst_buffer == NULL)
	{
		printf("unable to malloc memory for udpate_fdt \n");
		goto _err_out;
	}
	memset(file_dst_buffer,0, file1_len_align+file2_len_align);
	memcpy(file_dst_buffer, file1_buffer, file1_len);
	memcpy(file_dst_buffer+file1_len_align, file2_buffer, file2_len);
	
	file3 = fopen(file_name3, "wb");
	if(!file3)
	{
		printf("unable to open file %s\n", file_name3);
		goto _err_out;
	} 
	fwrite(file_dst_buffer, file1_len_align+file2_len_align, 1, file3);
	fclose(file3); file3 = NULL;
	
	printf("merge_uboot:genrate %s ok\n",file_name3);
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
