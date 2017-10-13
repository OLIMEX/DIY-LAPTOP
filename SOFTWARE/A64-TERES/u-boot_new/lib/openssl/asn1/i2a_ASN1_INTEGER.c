#include"asn1.h"
#include"bio.h"
#include"err.h"

///////////////BIO_write////////////////////ok

int BIO_write(BIO *b, const void *in, int inl)
	{
	int i;
	long (*cb)(BIO *,int,const char *,int,long,long);

	if (b == NULL)
		return(0);

	cb=b->callback;
	if ((b->method == NULL) || (b->method->bwrite == NULL))
		{
		return(-2);
		}

	if ((cb != NULL) &&
		((i=(int)cb(b,BIO_CB_WRITE,in,inl,0L,1L)) <= 0))
			return(i);

	if (!b->init)
		{
		return(-2);
		}

	i=b->method->bwrite(b,in,inl);

	if (i > 0) b->num_write+=(unsigned long)i;

	if (cb != NULL)
		i=(int)cb(b,BIO_CB_WRITE|BIO_CB_RETURN,in,inl,
			0L,(long)i);
	return(i);
	}


///////////i2a_ASN1_INTEGER////////////////////ok

int i2a_ASN1_INTEGER(BIO *bp, ASN1_INTEGER *a)
	{
	int i,n=0;
	static const char *h="0123456789ABCDEF";
	char buf[2];

	if (a == NULL) return(0);

	if (a->type & V_ASN1_NEG)
		{
		if (BIO_write(bp, "-", 1) != 1) goto err;
		n = 1;
		}

	if (a->length == 0)
		{
		if (BIO_write(bp,"00",2) != 2) goto err;
		n += 2;
		}
	else
		{
		for (i=0; i<a->length; i++)
			{
			if ((i != 0) && (i%35 == 0))
				{
				if (BIO_write(bp,"\\\n",2) != 2) goto err;
				n+=2;
				}
			buf[0]=h[((unsigned char)a->data[i]>>4)&0x0f];
			buf[1]=h[((unsigned char)a->data[i]   )&0x0f];
			if (BIO_write(bp,buf,2) != 2) goto err;
			n+=2;
			}
		}
	return(n);
err:
	return(-1);
	}
