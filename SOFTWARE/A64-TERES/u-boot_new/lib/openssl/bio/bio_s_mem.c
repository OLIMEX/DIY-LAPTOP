#include"bio.h"
#include"buffer.h"
#include"cryptlib.h"
#include"err.h"
#include"myfunction.h"

static int mem_write(BIO *h, const char *buf, int num);
static int mem_read(BIO *h, char *buf, int size);
static int mem_puts(BIO *h, const char *str);
static int mem_gets(BIO *h, char *str, int size);
static long mem_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int mem_new(BIO *h);
static int mem_free(BIO *data);

//static BIO_METHOD mem_method=	//--hgl--20140331--RW mem to const mem
const BIO_METHOD mem_method=
	{
	BIO_TYPE_MEM,
	"memory buffer",
	mem_write,
	mem_read,
	mem_puts,
	mem_gets,
	mem_ctrl,
	mem_new,
	mem_free,
	NULL,
};

//////////////BIO_s_mem/////////////////////ok

BIO_METHOD *BIO_s_mem(void)
{

   return(BIO_METHOD *)(&mem_method);
}
///////////////BUF_MEM_new/////////////////////////ok

BUF_MEM *BUF_MEM_new(void)
	{
	BUF_MEM *ret;
	
	ret=OPENSSL_malloc(sizeof(BUF_MEM));
	if (ret == NULL)
		{
		BUFerr(BUF_F_BUF_MEM_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->length=0;
	ret->max=0;
	ret->data=NULL;
	return(ret);
	}

//////////////// BUF_MEM_free////////////////////////ok

void BUF_MEM_free(BUF_MEM *a)
	{
		
	if(a == NULL)
	    return;
	if (a->data != NULL)
		{
		memset(a->data,0,(unsigned int)a->max);
		OPENSSL_free(a->data);
		}
	OPENSSL_free(a);
	}


/////////////////BUF_MEM_grow_clean/////////////////////////ok

int BUF_MEM_grow_clean(BUF_MEM *str, int len)
	{
	char *ret;
	unsigned int n;
	
	if (str->length >= len)
		{
		memset(&str->data[len],0,str->length-len);
		str->length=len;
		return(len);
		}
	if (str->max >= len)
		{
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		return(len);
		}
	n=(len+3)/3*4;
	if (str->data == NULL)
		ret=OPENSSL_malloc(n);
	else
		ret=OPENSSL_realloc_clean(str->data,str->max,n);
	if (ret == NULL)
		{
		BUFerr(BUF_F_BUF_MEM_GROW_CLEAN,ERR_R_MALLOC_FAILURE);
		len=0;
		}
	else
		{
		str->data=ret;//这里是存放数据
		str->max=n;
		memset(&str->data[str->length],0,len-str->length);
		str->length=len;
		}
	return(len);
	}
//////////////////BIO_clear_flags///////////////////////////ok

void BIO_clear_flags(BIO *b, int flags)
	{
	b->flags &= ~flags;
	}
//////////////BIO_set_flags/////////////////////

void BIO_set_flags(BIO *b, int flags)
	{
	b->flags |= flags;
	}

/////////////mem_new///////////////////ok
static int mem_new(BIO *bi)
	{
	BUF_MEM *b;
	if ((b=BUF_MEM_new()) == NULL)
		return(0);
	bi->shutdown=1;
	bi->init=1;
	bi->num= -1;
	bi->ptr=(char *)b;
	return(1);
	}

////////////////mem_free///////////////////////ok

static int mem_free(BIO *a)
	{
	
	if (a == NULL) return(0);
	if (a->shutdown)
		{
		if ((a->init) && (a->ptr != NULL))
			{
			BUF_MEM *b;
			b = (BUF_MEM *)a->ptr;
			if(a->flags & BIO_FLAGS_MEM_RDONLY) b->data = NULL;
			BUF_MEM_free(b);
			a->ptr=NULL;
			}
		}
	return(1);
	}

///////////////// mem_read//////////////////////////ok

static int mem_read(BIO *b, char *out, int outl)
	{
	int ret= -1;
	BUF_MEM *bm;
	int i;
	char *from,*to;
	
	bm=(BUF_MEM *)b->ptr;
	BIO_clear_retry_flags(b);
	ret=(outl > bm->length)?bm->length:outl;
	if ((out != NULL) && (ret > 0)) {
		memcpy(out,bm->data,ret);
		bm->length-=ret;
		/* memmove(&(bm->data[0]),&(bm->data[ret]), bm->length); */
		if(b->flags & BIO_FLAGS_MEM_RDONLY) bm->data += ret;
		else {
			from=(char *)&(bm->data[ret]);
			to=(char *)&(bm->data[0]);
			for (i=0; i<bm->length; i++)
				to[i]=from[i];
		}
	} else if (bm->length == 0)
		{
		ret = b->num;
		if (ret != 0)
			BIO_set_retry_read(b);
		}
	return(ret);
	}

///////////////mem_write////////////////////////////////ok

static int mem_write(BIO *b, const char *in, int inl)
	{
	int ret= -1;
	int blen;
	BUF_MEM *bm;
	
	bm=(BUF_MEM *)b->ptr;
	if (in == NULL)
		{
		BIOerr(BIO_F_MEM_WRITE,BIO_R_NULL_PARAMETER);
		goto end;
		}

	if(b->flags & BIO_FLAGS_MEM_RDONLY) {
		BIOerr(BIO_F_MEM_WRITE,BIO_R_WRITE_TO_READ_ONLY_BIO);
		goto end;
	}

	BIO_clear_retry_flags(b);
	blen=bm->length;
	if (BUF_MEM_grow_clean(bm,blen+inl) != (blen+inl))
		goto end;
	memcpy(&(bm->data[blen]),in,inl);
	ret=inl;
end:
	return(ret);
	}


static long mem_ctrl(BIO *b, int cmd, long num, void *ptr)
{
	return 0;
}

static int mem_gets(BIO *bp, char *buf, int size)
{
   return 0;
}

static int mem_puts(BIO *bp, const char *str)
{
	return 0;
}
