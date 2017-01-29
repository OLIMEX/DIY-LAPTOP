#include <linux/string.h>
#include <asm/unistd.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>

#include "ini.h"


char CFG_SSL = '[';  /* 项标志符Section Symbol --可根据特殊需要进行定义更改，如 { }等*/
char CFG_SSR = ']';  /* 项标志符Section Symbol --可根据特殊需要进行定义更改，如 { }等*/
char CFG_NIS = ':';  /* name 与 index 之间的分隔符 */
char CFG_NTS = '#';  /* 注释符*/

static char * ini_str_trim_r(char * buf);
static char * ini_str_trim_l(char * buf);
static int ini_file_get_line(char *filedata, char *buffer, int maxlen); 
static int ini_split_key_value(char *buf, char **key, char **val); 
static long atol(char *nptr);


/*************************************************************
Function: 获得key的值
Input: char * filedata　文件；char * section　项值；char * key　键值
Output: char * value　key的值
Return: 0		SUCCESS
		-1		未找到section
		-2		未找到key
		-10		文件打开失败
		-12		读取文件失败
		-14		文件格式错误
		-22		超出缓冲区大小
Note: 
*************************************************************/
int ini_get_key(char *filedata, char * section, char * key, char * value)
{
	char *buf1, *buf2;
	char *key_ptr, *val_ptr;
	int  n, ret;
	int dataoff = 0;
	int i = 0;
	
	buf1 = (char *)vmalloc(MAX_CFG_BUF + 1);
	if (buf1 == NULL)
		return -ENOMEM;
	buf2 = (char *)vmalloc(MAX_CFG_BUF + 1); {
		vfree(buf1);
		return -ENOMEM;
	}
	
	*value='\0';
	
	while(i<5) { /* 搜找项section */
		ret = CFG_ERR_READ_FILE;
		n = ini_file_get_line(filedata+dataoff, buf1, MAX_CFG_BUF);
		dataoff += n;
		if(n < -1)
			goto r_cfg_end;
		ret = CFG_SECTION_NOT_FOUND;
		if(n < 0)
			goto r_cfg_end; /* 文件尾，未发现 */ 

		n = strlen(ini_str_trim_l(ini_str_trim_r(buf1)));
		if(n == 0 || buf1[0] == CFG_NTS)
			continue;       /* 空行 或 注释行 */ 

		ret = CFG_ERR_FILE_FORMAT;
		if(n > 2 && ((buf1[0] == CFG_SSL && buf1[n-1] != CFG_SSR)))
			goto r_cfg_end;
		if(buf1[0] == CFG_SSL) {
			buf1[n-1] = 0x00;
			if(strcmp(buf1+1, section) == 0) 
				break; /* 找到项section */
		} 
		i++;
	} 
	i = 0;
	while(1){ /* 搜找key */ 
		ret = CFG_ERR_READ_FILE;
		n = ini_file_get_line(filedata+dataoff, buf1, MAX_CFG_BUF);
		dataoff += n;
		if(n < -1) 
			goto r_cfg_end;
		ret = CFG_KEY_NOT_FOUND;
		if(n < 0)
			goto r_cfg_end;/* 文件尾，未发现key */ 

		n = strlen(ini_str_trim_l(ini_str_trim_r(buf1))); 
		if(n == 0 || buf1[0] == CFG_NTS) 
			continue;       /* 空行 或 注释行 */ 
		ret = CFG_KEY_NOT_FOUND; 
		if(buf1[0] == CFG_SSL) 
			goto r_cfg_end; 
		if(buf1[n-1] == '+') { /* 遇+号表示下一行继续  */ 		
			buf1[n-1] = 0x00; 
			while(1) {			
				ret = CFG_ERR_READ_FILE; 
				n = ini_file_get_line(filedata+dataoff, buf2, MAX_CFG_BUF);
				dataoff += n;
				if(n < -1) 
					goto r_cfg_end; 
				if(n < 0) 
					break;/* 文件结束 */ 

				n = strlen(ini_str_trim_r(buf2)); 
				ret = CFG_ERR_EXCEED_BUF_SIZE; 
				if(n > 0 && buf2[n-1] == '+'){/* 遇+号表示下一行继续 */ 
				 	buf2[n-1] = 0x00; 
					if( (strlen(buf1) + strlen(buf2)) > MAX_CFG_BUF) 
						goto r_cfg_end; 
					strcat(buf1, buf2); 
					continue; 
				} 
				if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF) 
					goto r_cfg_end; 
				strcat(buf1, buf2); 
				break; 
			} 
		} 
		ret = CFG_ERR_FILE_FORMAT; 
		if(ini_split_key_value(buf1, &key_ptr, &val_ptr) != 1) 
			goto r_cfg_end; 
		ini_str_trim_l(ini_str_trim_r(key_ptr)); 
		if(strcmp(key_ptr, key) != 0) 
			continue;                                  /* 和key值不匹配 */ 
		strcpy(value, val_ptr); 
		break; 
	} 
	ret = CFG_OK; 
r_cfg_end: 
	//if(fp != NULL) fclose(fp); 
	vfree(buf1);
	vfree(buf2);
	return ret; 
} 
/*************************************************************
Function: 获得所有section
Input:  char *filename　文件,int max 最大可返回的section的个数
Output: char *sections[]　存放section名字
Return: 返回section个数。若出错，返回负数。
		-10			文件打开出错
		-12			文件读取错误
		-14			文件格式错误
Note: 
*************************************************************/
int ini_get_sections(char *filedata, unsigned char * sections[], int max)
{
	//FILE *fp; 
	char buf1[MAX_CFG_BUF + 1]; 
	int n, n_sections = 0, ret; 
	int dataoff = 0;
	int i = 0;
	
//	if((fp = fopen(filename, "rb")) == NULL) 
//		return CFG_ERR_OPEN_FILE; 
	
	while(i<5) {/*搜找项section */
		ret = CFG_ERR_READ_FILE;
		n = ini_file_get_line(filedata+dataoff, buf1, MAX_CFG_BUF);
		dataoff += n;
		if(n < -1) 
			goto cfg_scts_end; 
		if(n < 0)
			break;/* 文件尾 */ 
		n = strlen(ini_str_trim_l(ini_str_trim_r(buf1)));
		if(n == 0 || buf1[0] == CFG_NTS) 
			continue;       /* 空行 或 注释行 */ 
		ret = CFG_ERR_FILE_FORMAT;
		if(n > 2 && ((buf1[0] == CFG_SSL && buf1[n-1] != CFG_SSR)))
			goto cfg_scts_end;
		if(buf1[0] == CFG_SSL) {
			if (max!=0){
				buf1[n-1] = 0x00;
				strcpy((char *)sections[n_sections], buf1+1);
				if (n_sections>=max)
					break;		/* 超过可返回最大个数 */
			}
			n_sections++;
		} 
		i++;

	} 
	ret = n_sections;
cfg_scts_end: 
//	if(fp != NULL)
//		fclose(fp);
	return ret;
} 


/*************************************************************
Function: 去除字符串右边的空字符
Input:  char * buf 字符串指针
Output: 
Return: 字符串指针
Note: 
*************************************************************/
static char * ini_str_trim_r(char * buf)
{
	int len,i;
	char tmp[128];

	memset(tmp, 0, sizeof(tmp));
	len = strlen(buf);
//	tmp = (char *)malloc(len);
	
	memset(tmp,0x00,len);
	for(i = 0;i < len;i++) {
		if (buf[i] !=' ')
			break;
	}
	if (i < len) {
		strncpy(tmp,(buf+i),(len-i));
	}
	strncpy(buf,tmp,len);
//	free(tmp);
	return buf;
}

/*************************************************************
Function: 去除字符串左边的空字符
Input:  char * buf 字符串指针
Output: 
Return: 字符串指针
Note: 
*************************************************************/
static char * ini_str_trim_l(char * buf)
{
	int len,i;	
	char tmp[128];

	memset(tmp, 0, sizeof(tmp));
	len = strlen(buf);
	//tmp = (char *)malloc(len);

	memset(tmp,0x00,len);

	for(i = 0;i < len;i++) {
		if (buf[len-i-1] !=' ')
			break;
	}
	if (i < len) {
		strncpy(tmp,buf,len-i);
	}
	strncpy(buf,tmp,len);
	//free(tmp);
	return buf;
}
/*************************************************************
Function: 从文件中读取一行
Input:  FILE *fp 文件句柄；int maxlen 缓冲区最大长度
Output: char *buffer 一行字符串
Return: >0		实际读的长度
		-1		文件结束
		-2		读文件出错
Note: 
*************************************************************/
static int ini_file_get_line(char *filedata, char *buffer, int maxlen)
{
	int  i, j; 
	char ch1; 
	
	for(i=0, j=0; i<maxlen; j++) { 
		ch1 = filedata[j];
		if(ch1 == '\n' || ch1 == 0x00) 
			break; /* 换行 */ 
		if(ch1 == '\f' || ch1 == 0x1A) {      /* '\f':换页符也算有效字符 */ 			
			buffer[i++] = ch1; 
			break; 
		}
		if(ch1 != '\r') buffer[i++] = ch1;    /* 忽略回车符 */ 
	} 
	buffer[i] = '\0'; 
	return i+2; 
} 
/*************************************************************
Function: 分离key和value
			key=val
			jack   =   liaoyuewang 
			|      |   | 
			k1     k2  i 
Input:  char *buf
Output: char **key, char **val
Return: 1 --- ok 
		0 --- blank line 
		-1 --- no key, "= val" 
		-2 --- only key, no '=' 
Note: 
*************************************************************/
static int  ini_split_key_value(char *buf, char **key, char **val)
{
	int  i, k1, k2, n; 
	
	if((n = strlen((char *)buf)) < 1)
		return 0; 
	for(i = 0; i < n; i++) 
		if(buf[i] != ' ' && buf[i] != '\t')
			break; 

	if(i >= n)
		return 0;

	if(buf[i] == '=')
		return -1;
	
	k1 = i;
	for(i++; i < n; i++) 
		if(buf[i] == '=') 
			break;

	if(i >= n)
		return -2;
	k2 = i;
	
	for(i++; i < n; i++)
		if(buf[i] != ' ' && buf[i] != '\t') 
			break; 

	buf[k2] = '\0'; 

	*key = buf + k1; 
	*val = buf + i; 
	return 1; 
} 

int my_atoi(const char *str)
{
	int result = 0;
	int signal = 1; /* 默认为正数 */
	if((*str>='0'&&*str<='9')||*str=='-'||*str=='+') {
		if(*str=='-'||*str=='+') { 
			if(*str=='-')
				signal = -1; /*输入负数*/
			str++;
		}
	}
	else 
		return 0;
	/*开始转换*/
	while(*str>='0' && *str<='9')
	   result = result*10 + (*str++ - '0' );
	
	return signal*result;
}

int isspace(int x)  
{  
    if(x==' '||x=='\t'||x=='\n'||x=='\f'||x=='\b'||x=='\r')  
        return 1;  
    else   
        return 0;  
}  
  
int isdigit(int x)  
{  
    if(x<='9' && x>='0')           
        return 1;   
    else   
        return 0;  
  
} 

static long atol(char *nptr)
{
	int c; /* current char */
	long total; /* current total */
	int sign; /* if ''-'', then negative, otherwise positive */
	/* skip whitespace */
	while ( isspace((int)(unsigned char)*nptr) )
		++nptr;
	c = (int)(unsigned char)*nptr++;
	sign = c; /* save sign indication */
	if (c == '-' || c == '+')
		c = (int)(unsigned char)*nptr++; /* skip sign */
	total = 0;
	while (isdigit(c)) {
		total = 10 * total + (c - '0'); /* accumulate digit */
		c = (int)(unsigned char)*nptr++; /* get next char */
	}
	if (sign == '-')
		return -total;
	else
		return total; /* return result, negated if necessary */
}
/***
*int atoi(char *nptr) - Convert string to long
*
*Purpose:
* Converts ASCII string pointed to by nptr to binary.
* Overflow is not detected. Because of this, we can just use
* atol().
*
*Entry:
* nptr = ptr to string to convert
*
*Exit:
* return int value of the string
*
*Exceptions:
* None - overflow is not detected.
*
*******************************************************************************/
int atoi(char *nptr)
{
	return (int)atol(nptr);
}

