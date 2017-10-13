#include"bn_lcl.h"
#include"buffer.h"
#include"bn.h"

/////////////////// BN_bn2bin////////////////////////////////////////ok

int BN_bn2bin(const BIGNUM *a, unsigned char *to)
	{
	int n,i;
	BN_ULONG l;
	
	bn_check_top(a);
	n=i=BN_num_bytes(a);
	while (i--)
		{
		l=a->d[i/BN_BYTES];
		*(to++)=(unsigned char)(l>>(8*(i%BN_BYTES)))&0xff;
		}
	return(n);
	}

////////////////BN_bin2bn/////////////////////////////////////////////////ok

BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret)
	{
	unsigned int i,m;
	unsigned int n;
	BN_ULONG l;
	BIGNUM  *bn = NULL;
	
	if (ret == NULL)
		ret = bn = BN_new();
	if (ret == NULL) return(NULL);
	bn_check_top(ret);
	l=0;
	n=len;
	if (n == 0)
		{
		ret->top=0;
		return(ret);
		}
	i=((n-1)/BN_BYTES)+1;
	m=((n-1)%(BN_BYTES));
	if (bn_wexpand(ret, (int)i) == NULL)
		{
		//if (bn) BN_free(bn);
		return NULL;
		}
	ret->top=i;
	ret->neg=0;
	while (n--)
		{
		l=(l<<8L)| *(s++);
		if (m-- == 0)
			{
			ret->d[--i]=l;
			l=0;
			m=BN_BYTES-1;
			}
		}

	bn_correct_top(ret);
	return(ret);
	}

