#include "asn1.h"
#include "asn1_mac.h"
#include "limits.h"
#include "err.h"

static void asn1_put_length(unsigned char **pp, int length);
static int asn1_get_length(const unsigned char **pp,int *inf,long *rl,int max);

/////////////ASN1_put_object/////////////////////////////////////////ok

void ASN1_put_object(unsigned char **pp, int constructed, int length, int tag,
	     int xclass)
{
	unsigned char *p= *pp;
	int i, ttag;

	i=(constructed)?V_ASN1_CONSTRUCTED:0;
	i|=(xclass&V_ASN1_PRIVATE);
	if (tag < 31)
		*(p++)=i|(tag&V_ASN1_PRIMITIVE_TAG);
	else
		{
		*(p++)=i|V_ASN1_PRIMITIVE_TAG;
		for(i = 0, ttag = tag; ttag > 0; i++) ttag >>=7;
		ttag = i;
		while(i-- > 0)
			{
			p[i] = tag & 0x7f;
			if(i != (ttag - 1)) p[i] |= 0x80;
			tag >>= 7;
			}
		p += ttag;
		}
	if (constructed == 2)
		*(p++)=0x80;
	else
		asn1_put_length(&p,length);
	*pp=p;
}

//////////////asn1_put_length///////////////////////////////////////ok

static void asn1_put_length(unsigned char **pp, int length)
	{
	unsigned char *p= *pp;
	int i,l;

	if (length <= 127)
		*(p++)=(unsigned char)length;
	else
		{
		l=length;
		for (i=0; l > 0; i++)
			l>>=8;
		*(p++)=i|0x80;
		l=i;
		while (i-- > 0)
			{
			p[i]=length&0xff;
			length>>=8;
			}
		p+=l;
		}
	*pp=p;
	}


////////////////ASN1_object_size//////////////////////////////////ok

int ASN1_object_size(int constructed, int length, int tag)
	{
	int ret;

	ret=length;
	ret++;
	if (tag >= 31)
		{
		while (tag > 0)
			{
			tag>>=7;
			ret++;
			}
		}
	if (constructed == 2)
		return ret + 3;
	ret++;
	if (length > 127)
		{
		while (length > 0)
			{
			length>>=8;
			ret++;
			}
		}
	return(ret);
	}

/////////ASN1_get_object//////////////////////////////////////////////ok
//	--YXY	获得证书是什么结构类型，例如sequence
int ASN1_get_object(const unsigned char **pp, long *plength, int *ptag,
	int *pclass, long omax)
	{
	int i,ret;
	long l;
	const unsigned char *p= *pp;
	int tag,xclass,inf;
	long max=omax;

	if (!max) goto err;
	ret=(*p&V_ASN1_CONSTRUCTED);//0x20
	xclass=(*p&V_ASN1_PRIVATE);//0xc0
	i= *p&V_ASN1_PRIMITIVE_TAG;//0x1f
	if (i == V_ASN1_PRIMITIVE_TAG)
		{		/* high-tag */
		p++;
		if (--max == 0) goto err;
		l=0;
		while (*p&0x80)
			{
			l<<=7L;
			l|= *(p++)&0x7f;
			if (--max == 0) goto err;
			if (l > (INT_MAX >> 7L)) goto err;
			}
		l<<=7L;
		l|= *(p++)&0x7f;
		tag=(int)l;
		if (--max == 0) goto err;
		}
	else
		{
		tag=i;//确定是什么类型,即universal的值
		p++;
		if (--max == 0) goto err;
		}
	*ptag=tag;
	*pclass=xclass;
	if (!asn1_get_length(&p,&inf,plength,(int)max)) goto err;//intenger

	if (*plength > (omax - (p - *pp)))//判断证书剩余的大小
		{
		ASN1err(ASN1_F_ASN1_GET_OBJECT,ASN1_R_TOO_LONG);
		ret|=0x80;
		}
	*pp=p;//证书的偏移地址
	return(ret|inf);
err:
	ASN1err(ASN1_F_ASN1_GET_OBJECT,ASN1_R_HEADER_TOO_LONG);
	return(0x80);
	}

/////////////////asn1_get_length//////////////////////////////////////////ok
//--YXY	获取证书的结构中的长度
static int asn1_get_length(const unsigned char **pp, int *inf, long *rl, int max)
	{
	const unsigned char *p= *pp;
	unsigned long ret=0;
	unsigned int i;

	if (max-- < 1) return(0);
	if (*p == 0x80)
		{
		*inf=1;
		ret=0;
		p++;
		}
	else
		{
		*inf=0;
		i= *p&0x7f;//获得多少个字节0x82，即2个字节
		if (*(p++) & 0x80)//获得长度0x04,0x51,即1105
			{
			if (i > sizeof(long))
				return 0;
			if (max-- == 0) return(0);
			while (i-- > 0)
				{
				ret<<=8L;
				ret|= *(p++);
				if (max-- == 0) return(0);
				}
			}
		else
			ret=i;
		}
	if (ret > LONG_MAX)
		return 0;
	*pp=p;//返回证书的偏移地址
	*rl=(long)ret;//返回长度值



	return(1);
	}

