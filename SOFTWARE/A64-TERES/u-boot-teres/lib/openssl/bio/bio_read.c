#include"bio.h"
#include"errno.h"
#include"err.h"
#include"cryptlib.h"
#include"bio_lcl.h"

#define MS_CALLBACK			//samyang  modify

////////////////BIO_read/////////////////////////ok

int BIO_read(BIO *b, void *out, int outl)
	{
	int i;
	long (*cb)(BIO *,int,const char *,int,long,long);
	
	if ((b == NULL) || (b->method == NULL) || (b->method->bread == NULL))
		{
		BIOerr(BIO_F_BIO_READ,BIO_R_UNSUPPORTED_METHOD);
		return(-2);
		}

	cb=b->callback;
	if ((cb != NULL) &&
		((i=(int)cb(b,BIO_CB_READ,out,outl,0L,1L)) <= 0))
			return(i);

	if (!b->init)
		{
		BIOerr(BIO_F_BIO_READ,BIO_R_UNINITIALIZED);
		return(-2);
		}

	i=b->method->bread(b,out,outl);

	if (i > 0) b->num_read+=(unsigned long)i;

	if (cb != NULL)
		i=(int)cb(b,BIO_CB_READ|BIO_CB_RETURN,out,outl,
			0L,(long)i);
	return(i);
	}

