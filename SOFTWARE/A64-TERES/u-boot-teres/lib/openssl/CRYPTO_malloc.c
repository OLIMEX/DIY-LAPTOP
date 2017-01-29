#include"crypto.h"
#include"cryptlib.h"
#include"lhash.h"
#include"myfunction.h"


unsigned char cleanse_ctr = 0;

void OPENSSL_cleanse(void *ptr, size_t len)
{
	unsigned char *p = ptr;
	size_t loop = len, ctr = cleanse_ctr;
	while(loop--)
		{
		*(p++) = (unsigned char)ctr;
		ctr += (17 + ((size_t)p & 0xF));
		}
	p=memchr(ptr, (unsigned char)ctr, len);
	if(p)
		ctr += (63 + (size_t)p);
	cleanse_ctr = (unsigned char)ctr;
}


void *CRYPTO_malloc(int num, const char *file, int line)
	{
	if (num <= 0) return NULL;

	return malloc(num);
	}

void *CRYPTO_realloc(void *str, int num, const char *file, int line)
	{
	if (str == NULL)
		return CRYPTO_malloc(num, file, line);

	if (num <= 0) return NULL;

	return realloc(str, num);
	}

void *CRYPTO_realloc_clean(void *str, int old_len, int num, const char *file,
			   int line)
	{
	void *ret = NULL;

	if (str == NULL)
		return CRYPTO_malloc(num, file, line);

	if (num <= 0) return NULL;

	/* We don't support shrinking the buffer. Note the memcpy that copies
	 * |old_len| bytes to the new buffer, below. */
	if (num < old_len) return NULL;

	ret =  malloc(num);
	if(ret)
		{
		memcpy(ret,str,old_len);
		OPENSSL_cleanse(str,old_len);
		free(str);
		}

	return ret;
	}

void CRYPTO_free(void *str)
	{
	free(str);
	}

void *CRYPTO_remalloc(void *a, int num, const char *file, int line)
	{
	if (a != NULL) free(a);
	a=(char *)malloc(num);
	return(a);
	}


void reset_CRYPTO_reset(void)
{
	cleanse_ctr = 0;
}
