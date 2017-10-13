// script.cpp : Defines the entry point for the console application.
//
#include <malloc.h>
#include <string.h>
#include "types.h"
#include "script.h"
#include <ctype.h>
#include <unistd.h>

__asm__(".symver memcpy ,memcpy@GLIBC_2.2.5");

int parser_script(void *pbuf, int script_len, FILE *hfile);
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int IsFullName(const char *FilePath)
{
    if ( FilePath[0] == '/')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void GetFullPath(char *dName, const char *sName)
{
    char Buffer[MAX_PATH];

	if(IsFullName(sName))
	{
	    strcpy(dName, sName);
		return ;
	}

   /* Get the current working directory: */
   if(getcwd(Buffer, MAX_PATH ) == NULL)
   {
        perror( "_getcwd error" );
        return ;
   }
   sprintf(dName, "%s/%s", Buffer, sName);
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void Usage(void)
{
	printf("\n");
	printf("Usage:\n");
	printf("script.exe script file path para file path\n\n");
}
static int _get_str_length(char *str)
{
	int length = 0;

	while(str[length])
	{
		length ++;
	}

	return length;
}
//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	//char str1[] = "D:\\winners\\eBase\\eGON\\eGON2\\workspace\\sun_20\\eFex\\sys_config1.fex";
	//char str2[] = "D:\\winners\\eBase\\eGON\\eGON2\\workspace\\sun_20\\eFex\\sys_config.bin";
	char str1[] = "sys_config.fex";
	char str2[] = "sys_config.bin";
	char   src[MAX_PATH];
	char   dest[MAX_PATH];
	FILE  *dst_file = NULL;
	FILE  *script_file = NULL;
	int    ret = -1, src_length;
	char   *script_addr = NULL;
	int    script_len, i;

	printf("argc = %d\n", argc);
	for(i=1;i<argc;i++)
	{
		if(argv[i] == NULL)
		{
			printf("script error: input source file name is empty\n");

			return __LINE__;
		}
		printf("input name %s\n", argv[i]);
		memset(src, 0, sizeof(MAX_PATH));
		memset(dest, 0, sizeof(MAX_PATH));
		GetFullPath(src, argv[i]);

		src_length = _get_str_length(src);
		memcpy(dest, src, src_length);
		dest[src_length - 0] = NULL;
		dest[src_length - 1] = 'n';
		dest[src_length - 2] = 'i';
		dest[src_length - 3] = 'b';

		printf("Script %d source file Path=%s\n", i, src);
		printf("Script %d bin file Path=%s\n", i, dest);

		//打开脚本文件，打不开则失败
		script_file = fopen(src, "rb");
		if(!script_file)
		{
			printf("unable to open script file %s\n", src);

			goto _err_out;
		}

		//打开目的文件，打不开则失败
		dst_file = fopen(dest, "wb");
		if(!dst_file)
		{
			printf("unable to open dest file\n");

			goto _err_out;
		}
		//读出脚本的数据
		//首先获取脚本的长度
		fseek(script_file, 0, SEEK_END);
		script_len = ftell(script_file);
		fseek(script_file, 0, SEEK_SET);
		//读出脚本所有的内容
		script_addr = (char *)malloc(script_len+1);
		memset(script_addr,0,script_len+1);
		if(!script_addr)
		{
			printf("unable to malloc memory for script\n");

			goto _err_out;
		}
		fread(script_addr, 1, script_len, script_file);
		fclose(script_file);
		script_file = NULL;
		//对顶格书写的中括号做匹配，认为是一个数据项，标准是，在回车符号后面出现的
		ret = parser_script(script_addr, script_len, dst_file);
		if(ret)
		{
			printf("error1\n");
			goto _err_out;
		}
		free(script_addr);
		script_addr = NULL;
		fclose(dst_file);
		dst_file = NULL;
		printf("parser %d file ok\n", i);
	}

_err_out:
	//退出时候的处理
	if(script_addr)
	{
		free(script_addr);
	}
	if(script_file)
	{
		fclose(script_file);
		script_file = NULL;
	}

	if(dst_file)
	{
		fclose(dst_file);
		dst_file = NULL;
	}

	return ret;
}
#define  THIS_LINE_NULL        (0)
#define  THIS_LINE_MAINKEY     (1)
#define  THIS_LINE_SUBKEY      (2)
#define  THIS_LINE_ERROR       (-1)
//此函数返回当前行的长度，用指针返回当前的意义
/********************************************
* flag = 0      //当前是注释行，或空行
*      = 1      //当前是字段行
*      = 2      //当前是子项行，属于字段的下一项
*      = -1     //当前行不符合规范，出错
*********************************************/
static  int  _get_line_status(char  *daddr,  int  *flag, int last_len)
{
    char  *src;
    int    len;
	char   ch;

    src = daddr;
	ch  = *src++;
	last_len --;
	switch(ch)
	{
		case ';':     //注释行
		case 0x0D:    //回车行
		{
			*flag = THIS_LINE_NULL;
		}
		break;

		case '[':    //主键行
		{
			*flag = THIS_LINE_MAINKEY;
			break;
		}

		default:     //子键行
		{
			*flag = THIS_LINE_SUBKEY;
			break;
		}
	}

    len = 1;
	ch = *src++;
    while((ch != 0x0A) && (last_len >  len))     //只要不是换行符号，继续查找
    {
		ch = *src++;
		len ++;
        if(len >= 512)
        {
            *flag = THIS_LINE_ERROR;

            return 0;
        }
    }

    return len + 1;
}

//查找出主键的字符串名称
static  int _fill_line_mainkey(char *pbuf, script_item_t *item)
{
	char *src;
	char  ch, i;

	i = 0;
	src = pbuf + 1;        //跳过 【
	ch  = *src++;

	while(']' != ch)
	{
		item->item_name[i] = ch;
		i++;
		ch = *src++;
		if(i >= ITEM_MAIN_NAME_MAX)
		{
			item->item_name[i-1] = 0;
			break;
		}
	}

	return 0;

}

static  int _get_item_value(char *pbuf, char *name, char *value)
{
    char  *src, *dstname, *dstvalue;
    int   len;

    src = pbuf;
    dstname = name;
    dstvalue = value;

    len = 0;
	//首先检查整行字符的合法性
	while(1)
	{
		//如果一开始遇到空格或者TAB，则移动指针，直到找到一个合法字符为止
		if((*src == ' ') || (*src == 0x09))
		{
			src ++;
		}
		//如果什么都没有找到，则直接返回
		else if((*src == 0x0D) || (*src == 0x0A))
		{
			dstname[0] = '\0';
			dstvalue[0] = '\0';

			return 0;
		}
		else
		{
			break;
		}
	}
	//已经找到一个合法字符，继续找，直到寻找到等号，去除尾部的空格或者TAB
    while(*src != '=')
    {
        dstname[len ++] = *src;
        src ++;
        if(len >= 31)
        {
			dstname[len] = '\0';
			break;
        }
    }
	while(1)
	{
		len --;
		if((dstname[len] == ' ') || (dstname[len] == 0x09))
		{
			dstname[len] = '\0';
		}
		else
		{
			dstname[len + 1] = '\0';
			break;
		}
	}

	while(*src != '=')
	{
		src ++;
	}

	src++;
    len = 0;
	//首先检查整行字符的合法性
	while(1)
	{
		//如果一开始遇到空格或者TAB，则移动指针，直到找到一个合法字符为止
		if((*src == ' ') || (*src == 0x09))
		{
			src ++;
		}
		//如果什么都没有找到，则直接返回
		else if((*src == 0x0D) || (*src == 0x0A))
		{
			dstvalue[0] = '\0';
			return 0;
		}
		else
		{
			break;
		}
	}
	//已经找到一个合法字符，继续找，直到寻找到等号，去除尾部的空格或者TAB
    while((*src != 0x0D) && (*src != 0x0A))
    {
        dstvalue[len ++] = *src;
        src ++;
        if(len >= 127)
        {
			dstvalue[len] = '\0';
			break;
        }
    }
	while(1)
	{
		len --;
		if((dstvalue[len] == ' ') || (dstvalue[len] == 0x09))
		{
			dstvalue[len] = '\0';
		}
		else
		{
			dstvalue[len + 1] = '\0';
			break;
		}
	}

    return 0;
}
//此函数转换字符数据称为整型数据，包括10进制和16进制
//转换结果存在在value中，返回值标志转换成功或者失败
static  int  _get_str2int(char *saddr, int value[] )
{
    char  *src;
    char   off, ch;
    unsigned int  tmp_value = 0;
	int    data_count, i;
	char   tmp_str[128];
	int    sign = 1;

	data_count = 2;
    src = saddr;
    off = 0;         //0代表10进制，1代表16进制

	if(!strncmp(src, "port:", 5))
	{
		if((src[5] == 'P') || (src[5] == 'p'))
		{
			off = 3;                    //表示是端口描述符号
			src += 6;
		}
	}
	else if(!strncmp(src, "string:", 7))
	{
		off = 0;
		src += 7;
	}
	else if(src[0] == '"')
	{
		off = 5;
		src += 1;
	}
	else if((src[0] == '0') && ((src[1] == 'x') || (src[1] == 'X')))      //十六进制
    {
        src += 2;
        off  = 2;
    }
	else if((src[0] >= '0') && (src[0] <= '9'))                     //十进制
	{
		off = 1;
	}
	else if(((src[1] >= '0') && (src[1] <= '9')) && (src[0] == '-'))
	{
		src ++;
		off = 1;
		sign = -1;
	}
	else if(src[0] == '\0')
	{
		src++;
		off = 4;
	}
	//表示是字符串
	if(off == 0)
	{
		data_count = 0;
		while(src[data_count] != '\0')
		{
			data_count ++;
			if(data_count > 127)
			{
				break;
			}
		}
		if(data_count & 0x03)       //要求四字节对齐
		{
			data_count = (data_count & (~0x03)) + 4;
		}
		else
		{
			data_count = data_count + 4;
		}
		value[0] = data_count>>2;
                if(saddr != src)
                {
		    value[1] = 0;
                }
                else
                {
                    value[1] = 1;
                }
		return DATA_TYPE_STRING;
	}
	else if(off == 5)	//表示是字符串
	{
		data_count = 0;
		while(src[data_count] != '"')
		{
			data_count ++;
			if(data_count > 127)
			{
				break;
			}
		}
		src[data_count] = '\0';
		if(data_count & 0x03)       //要求四字节对齐
		{
			data_count = (data_count & (~0x03)) + 4;
		}
		else
		{
			data_count = data_count + 4;
		}
		value[0] = data_count>>2;
		value[1] = 5;

		return DATA_TYPE_STRING;
	}
    else if(off == 1)
    {
        while(*src != '\0')
        {
            if((*src >= '0') && (*src <= '9'))
            {
                tmp_value = tmp_value * 10 + (*src - '0');
                src ++;
            }
            else
            {
                return -1;
            }
        }
		value[0] = tmp_value * sign;

		return DATA_TYPE_SINGLE_WORD;
    }
    else if(off == 2)
    {
        while(*src != '\0')
        {
			if((*src >= '0') && (*src <= '9'))
            {
                tmp_value = tmp_value * 16 + (*src - '0');
                src ++;
            }
            else if((*src >= 'A') && (*src <= 'F'))
            {
                tmp_value = tmp_value * 16 + (*src - 'A' + 10);
                src ++;
            }
            else if((*src >= 'a') && (*src <= 'f'))
            {
                tmp_value = tmp_value * 16 + (*src - 'a' + 10);
                src ++;
            }
            else
            {
                return -1;
            }
        }
		value[0] = tmp_value;

		return DATA_TYPE_SINGLE_WORD;
    }
	else if(off == 3)                              //表示是GPIO信息，必须按照标准格式对齐 端口编号：端口引脚编号：mult-driving：pull：driving-level：data共6个word
	{
		//获取字符编组信息：编组
		int  tmp_flag = 0;

		ch = *src++;
		if((ch == 'o') || (ch == 'O'))             //用1代表A组，2代表B组，依次类推，用0xffff代表POWER按键
		{
			ch = src[0];
			if((ch == 'w') || (ch == 'W'))
			{
				ch = src[1];
				if((ch == 'e') || (ch == 'E'))
				{
					ch = src[2];
					if((ch == 'r') || (ch == 'R'))
					{
						//确定，是POWER按键
						value[0] = 0xffff;
						src += 3;
						tmp_flag = 1;
					}
				}
			}
		}
		if(!tmp_flag)
		{
			if((ch >= 'A') && (ch <= 'Z'))
			{
				value[0] = ch - 'A' + 1;
			}
			else if((ch >= 'a') && (ch <= 'z'))
			{
				value[0] = ch - 'a' + 1;
			}
			else
			{
				return -1;
			}
		}

		//获取字符编组信息：组内序号
		//第一个版本，不支持自动获取参数
		ch = *src++;
		tmp_value = 0;
		while(ch != '<')
		{
			if((ch >= '0') && (ch <= '9'))
			{
				tmp_value = tmp_value * 10 + (ch - '0');
				ch = *src++;
			}
			else if(ch == 0)
			{
				src --;
				break;
			}
			else
			{
				return -1;
			}
		}
		value[1] = tmp_value;
		//开始遍历所有的尖括号
		ch = *src++;
		while(ch != '\0')
		{
			i = 0;
			memset(tmp_str, 0, sizeof(tmp_str));
			while(ch != '>')     //如果是数字
			{
				if((ch >= 'A') && (ch <= 'Z'))
				{
					ch += 'a' - 'A';
				}
				tmp_str[i++] = ch;
				ch = *src++;
			}
			tmp_str[i] = '\0';
			//比较字符串
			if(!strcmp(tmp_str, "default"))
			{
				value[data_count] = -1;
			}
			else if(!strcmp(tmp_str, "none"))
			{
				value[data_count] = -1;
			}
			else if(!strcmp(tmp_str, "null"))
			{
				value[data_count] = -1;
			}
			else if(!strcmp(tmp_str, "-1"))
			{
				value[data_count] = -1;
			}
			else
			{
				i = 0;
				ch = tmp_str[i++];
				tmp_value = 0;
				if(ch == '-')
				{
					sign = -1;
					ch = tmp_str[i++];
				}
				while(ch != '\0')
				{
					if((ch >= '0') && (ch <= '9'))
					{
						tmp_value = tmp_value * 10 + (ch - '0');
					}
					else
					{
						return -1;
					}
					ch = tmp_str[i++];
				}
				value[data_count] = tmp_value * sign;
			}

			data_count ++;
			ch = *src++;
			if(ch == '<')
			{
				ch = *src++;
			}
			else if(ch == '\0')
			{
				;
			}
			else
			{
				return -1;
			}
		}
		switch(data_count)
		{
			case 3:
				value[3] = -1;        // 上拉能力
			case 4:
				value[4] = -1;        // 驱动能力
			case 5:
				value[5] = -1;       // 输出端口
			case 6:
				break;
			default:
				return -1;
		}

		return DATA_TYPE_GPIO;
	}
	else if(off == 4)
	{
		value[0] = 4>>2;
		return DATA_EMPTY;
	}
	else
	{
		return -1;
	}
}
//算法：一行一行的查找
//      查找到冒号，直接认为是注释行，然后找到回车换行符号，找到下一行去    (语法规则)
//      查找到'['(左中括号)，认为是一个主键(记为A主键)，找出中括号中间的字符串，以及右中括号
//             找到下面的字符以及数据，依次存放在一块内存中
//      直到找到后面的主键(B主键)，然后认为A主键查找完毕
//      把A主键下的所有数据，按照从键名，键值的顺序排列，存放在另一个内存块中
//      记录A主键的长度(从键的个数)，记录A主键下的数据存放在内存中的偏移量

#define TIEM_MAIN_MAX       128

int parser_script(void *pbuf, int script_len, FILE *hfile)
{
	int   ret = -1;
	char  *src, *dest = NULL, *tmp_dest;
	int   *dest_data = NULL, *tmp_dest_data, dest_data_index;
	int   line_len, line_status;
	int   new_main_key_flag = 0, sub_value_index = 0;
	script_item_t   item_table[TIEM_MAIN_MAX];
	char  sub_name[128], sub_value[128];
	int   value[8];
	unsigned int i, main_key_index = 0;
	script_head_t   script_head;

	src = (char *)pbuf;
	//申请字段空间
	dest = (char *)malloc(512 * 1024);
	if(!dest)
	{
		printf("fail to get memory for script storage key\n");

		goto _err;
	}
	memset(dest, 0, 512 * 1024);
	tmp_dest = dest;
	//申请数据空间
	dest_data = (int *)malloc(512 * 1024);
	if(!dest_data)
	{
		printf("fail to get memory for script storage data\n");

		goto _err;
	}
	memset(dest_data, 0, 512 * 1024);
	dest_data_index = 0;
	tmp_dest_data = dest_data;

	memset(item_table, 0, TIEM_MAIN_MAX * sizeof(script_item_t));

	while(script_len)
	{
		line_len = _get_line_status(src, &line_status, script_len);//判断当前行的状态
		script_len -= line_len;

		switch(line_status)
		{
			case THIS_LINE_NULL:                                //注释行
			{
				break;
			}

			case THIS_LINE_MAINKEY:                             //主键行
			{
				if(_fill_line_mainkey(src, &item_table[main_key_index]))  //获取一个主键，立刻保存主键的相关信息
				{
					goto _err;
				}
				if(!new_main_key_flag)
				{
					new_main_key_flag = 1;
					item_table[main_key_index].item_offset = 0;
				}
				else
				{
					item_table[main_key_index].item_offset = item_table[main_key_index - 1].item_offset + item_table[main_key_index - 1].item_length * 10;
				}
				main_key_index ++;

				break;
			}

			case THIS_LINE_SUBKEY:                              //子键行，和前面的主键对应
			{
				if(!new_main_key_flag)
				{
					break;
				}
				if(_get_item_value(src, sub_name, sub_value))   //转换，把等号左方的存在到sub_nam，等号右方的存放到sub_value，都是字符串形式
				{
					goto _err;
				}

				//等号左方的直接放到内存中，按照最长32个字节的截取32个字节，不足32个字节的补齐后占用32个字节
				//后面4个字节用于存放对应数据的偏移量
				strcpy(tmp_dest, sub_name);
				tmp_dest += ITEM_MAIN_NAME_MAX;
				//右方数据存放到另一块内存中
				ret = _get_str2int(sub_value, value);
				if(ret == -1)           //数据错误
				{
					goto _err;
				}
				else if(ret == DATA_TYPE_SINGLE_WORD)           //数据正确，并且是普通数据，10进制或者16进制
				{
					*tmp_dest_data = value[0];
					*((unsigned int *)tmp_dest) = dest_data_index;
					tmp_dest += 4;
					*((unsigned int *)tmp_dest) = (1 << 0) | (DATA_TYPE_SINGLE_WORD << 16);       //该子键的长度1 word，类型为1，普通数据类型
					tmp_dest_data ++;
					dest_data_index ++;
				}
				else if(ret == DATA_EMPTY)
				{
					*((int *)tmp_dest) = dest_data_index;
					tmp_dest += 4;
					*((unsigned int *)tmp_dest) = (value[0] << 0) | (DATA_EMPTY << 16);       //该子键的长度value word，类型为2，字符串类型
					tmp_dest_data += value[0];
					dest_data_index += value[0];
				}
				else if(ret == DATA_TYPE_STRING)
				{
					*((int *)tmp_dest) = dest_data_index;
					if(value[1] == 0)
					{
						strncpy((char *)tmp_dest_data, sub_value + sizeof("string:") - 1, value[0]<<2);
					}
                                        else if(value[1] == 1)
                                        {
                                                strncpy((char *)tmp_dest_data,sub_value,value[0]<<2);
                                        }
					else
					{
						strncpy((char *)tmp_dest_data, sub_value + 1, value[0]<<2);
					}
					tmp_dest += 4;
					*((unsigned int *)tmp_dest) = (value[0] << 0) | (DATA_TYPE_STRING << 16);       //该子键的长度value word，类型为2，字符串类型
					tmp_dest_data += value[0];
					dest_data_index += value[0];
				}
				else if(ret == DATA_TYPE_GPIO)            //转换出多个字节数据
				{
					for(i=0;i<6;i++)
					{
						*(tmp_dest_data ++) = value[i];
					}
					*((unsigned int *)tmp_dest) = dest_data_index;
					tmp_dest += 4;
					*((unsigned int *)tmp_dest) = (6 << 0) | (DATA_TYPE_GPIO << 16);       //该子键的长度ret word，类型为3，多字节数据类型
					dest_data_index += 6;
				}
				else if(ret == DATA_TYPE_MULTI_WORD)
				{
					;
				}
				sub_value_index ++;
				tmp_dest += 4;
				item_table[main_key_index - 1].item_length ++;

				break;
			}

			default:
			{
				goto _err;
			}
		}
		src += line_len;
	}
	//保存所有的数据到文件中
	if(!main_key_index)
	{
		goto _err;
	}
	//校正主键的第一个子键的偏移，全部加上所有主键引起的偏移，注意是整型单位
	for(i=0;i<main_key_index;i++)
	{
		item_table[i].item_offset += ((sizeof(script_item_t) * main_key_index) >> 2) + (sizeof(script_head_t) >> 2);
	}
	//校正每个子键的偏移，全部加上所有主键引起偏移和所有子键名称引起的偏移，注意是整型单位
	{
		src = dest;
		i = 0;
		while(i < sub_value_index * 10 * sizeof(int))
		{
			src += ITEM_MAIN_NAME_MAX;
			*(unsigned int *)src += ((sizeof(script_item_t) * main_key_index) >> 2) + (sub_value_index * 10) + (sizeof(script_head_t)>>2);
			i += 10 * sizeof(int);
			src += 8;
		}
	}
	script_head.item_num = main_key_index;
	script_head.length   = sizeof(script_head_t) +  sizeof(script_item_t) * main_key_index +
	                       sub_value_index * 10 * sizeof(int) + dest_data_index * sizeof(int);
	script_head.version[0] = 1;
	script_head.version[1] = 2;
	fwrite(&script_head, 1, sizeof(script_head_t), hfile);
	fwrite(item_table, 1, sizeof(script_item_t) * main_key_index, hfile);      //保存所有的主键项目
	fwrite(dest, 1, sub_value_index * 10 * sizeof(int), hfile);                 //保存子键名称项目
	fwrite(dest_data, 1, dest_data_index * sizeof(int), hfile);                //保存所有的子键数值项目

_err:
	if(dest)
	{
		free(dest);
	}
	if(dest_data)
	{
		free(dest_data);
	}

	ret = ((ret >= 0) ? 0: -1);

	return ret;
}


