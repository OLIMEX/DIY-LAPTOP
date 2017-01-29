/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
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
void ndump(u8 *buf, int count)
{
	int i,j=0;
	int rest;
	char c;

	rest = count;
	if(count > 16)
	{
		for(j=0;j<count-16;j+=16)
		{
			for(i=0;i<16;i++)
			{
				c = buf[j+i] & 0xff;
				printf("%02x  ", c);
			}
			rest -= 16;
			printf("\n");
		}
	}
	for(i=0;i<rest;i++)
	{
		c = buf[j+i] & 0xff;
		printf("%02x  ", c);
	}

	printf("\n");
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
void xdump(u8 *buf, int count)
{
	int i,j=0;
	int rest;
	char c;

	rest = count;
	if(count > 16)
	{
		for(j=0;j<count-16;j+=16)
		{
			for(i=0;i<16;i++)
			{
				c = buf[j+i] & 0xff;
				printf("%02x", c);
				if((c > 20) && (c < 0x7e))
				{
					printf("(%c)  ", c);
				}
				else
				{
					printf("  ");
				}
			}
			rest -= 16;
			printf("\n");
		}
	}
	for(i=0;i<rest;i++)
	{
		c = buf[j+i] & 0xff;
		printf("%02x", c);
		if((c > 20) && (c < 0x7e))
		{
			printf("(%c)  ", c);
		}
		else
		{
			printf("  ");
		}
	}

	printf("\n");
}

