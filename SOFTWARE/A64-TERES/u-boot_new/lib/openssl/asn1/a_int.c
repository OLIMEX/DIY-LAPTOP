/* crypto/asn1/a_int.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */


#include <asn1.h>
#include <bn.h>
#include <err.h>

long ASN1_INTEGER_get(ASN1_INTEGER *a)
{
	int neg=0,i;
	long r=0;

	if (a == NULL) return(0L);
	i=a->type;
	if (i == V_ASN1_NEG_INTEGER)
		neg=1;
	else if (i != V_ASN1_INTEGER)
		return -1;

	if (a->length > (int)sizeof(long))
		{
		/* hmm... a bit ugly, return all ones */
		return -1;
		}
	if (a->data == NULL)
		return 0;

	for (i=0; i<a->length; i++)
		{
		r<<=8;
		r|=(unsigned char)a->data[i];
		}
	if (neg) r= -r;
	return(r);
}


int ASN1_STRING_mem(char *bp, const ASN1_STRING *v)
{
	int i,n;
	char *buf = bp;
	const char *p;

	if (v == NULL) return(0);
	n = 0;
	p=(const char *)v->data;
	for (i=0; i<v->length; i++)
		{
//		if ((p[i] > '~') || ((p[i] < ' ') &&
//			(p[i] != '\n') && (p[i] != '\r')))
//			buf[n++]='.';
//		else
			buf[n++]=p[i];
		}
	return(v->length);
}

ASN1_INTEGER *c2i_ASN1_INTEGER(ASN1_INTEGER **a, const unsigned char **pp,
	     long len)
{
	ASN1_INTEGER *ret=NULL;
	const unsigned char *p, *pend;
	unsigned char *to,*s;
	int i;

	if ((a == NULL) || ((*a) == NULL))
		{
		if ((ret=M_ASN1_INTEGER_new()) == NULL) return(NULL);	//interger
		ret->type=V_ASN1_INTEGER;
		}
	else
		ret=(*a);

	p= *pp;	//0x42c5cc?
	pend = p + len;
	s=(unsigned char *)OPENSSL_malloc((int)len+1);
	if (s == NULL)
		{
		i=ERR_R_MALLOC_FAILURE;
		goto err;
		}
	to=s;
	if(!len) {

		ret->type=V_ASN1_INTEGER;
	} else if (*p & 0x80) /* a negative number */
		{
		ret->type=V_ASN1_NEG_INTEGER;
		if ((*p == 0xff) && (len != 1)) {
			p++;
			len--;
		}
		i = len;
		p += i - 1;
		to += i - 1;
		while((!*p) && i) {
			*(to--) = 0;
			i--;
			p--;
		}

		if(!i) {
			*s = 1;
			s[len] = 0;
			len++;
		} else {
			*(to--) = (*(p--) ^ 0xff) + 1;
			i--;
			for(;i > 0; i--) *(to--) = *(p--) ^ 0xff;
		}
	} else {
		ret->type=V_ASN1_INTEGER;
		if ((*p == 0) && (len != 1))
			{
			p++;
			len--;
			}
		memcpy(s,p,(int)len);
	}

	if (ret->data != NULL) OPENSSL_free(ret->data);
	ret->data=s;
	ret->length=(int)len;
	if (a != NULL) (*a)=ret;
	*pp=pend;
	return(ret);
err:
	ASN1err(ASN1_F_C2I_ASN1_INTEGER,i);
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		M_ASN1_INTEGER_free(ret);
	return(NULL);
	}

///////////////////c2i_ASN1_BIT_STRING////////////////////////////////ok

ASN1_BIT_STRING *c2i_ASN1_BIT_STRING(ASN1_BIT_STRING **a,
	const unsigned char **pp, long len)
	{
	ASN1_BIT_STRING *ret=NULL;
	const unsigned char *p;
	unsigned char *s;
	int i;

	if (len < 1)
		{
		i=ASN1_R_STRING_TOO_SHORT;
		goto err;
		}

	if ((a == NULL) || ((*a) == NULL))
		{
		if ((ret=M_ASN1_BIT_STRING_new()) == NULL) return(NULL);
		}
	else
		ret=(*a);

	p= *pp;
	i= *(p++);

	ret->flags&= ~(ASN1_STRING_FLAG_BITS_LEFT|0x07); /* clear */
	ret->flags|=(ASN1_STRING_FLAG_BITS_LEFT|(i&0x07)); /* set */

	if (len-- > 1)
		{
		s=(unsigned char *)OPENSSL_malloc((int)len);
		if (s == NULL)
			{
			i=ERR_R_MALLOC_FAILURE;
			goto err;
			}
		memcpy(s,p,(int)len);
		s[len-1]&=(0xff<<i);
		p+=len;
		}
	else
		s=NULL;

	ret->length=(int)len;
	if (ret->data != NULL) OPENSSL_free(ret->data);
	ret->data=s;
	ret->type=V_ASN1_BIT_STRING;
	if (a != NULL) (*a)=ret;
	*pp=p;
	return(ret);
err:
	ASN1err(ASN1_F_C2I_ASN1_BIT_STRING,i);
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		M_ASN1_BIT_STRING_free(ret);
	return(NULL);
	}

////////////////c2i_ASN1_OBJECT//////////////////////////ok

ASN1_OBJECT *c2i_ASN1_OBJECT(ASN1_OBJECT **a, const unsigned char **pp,
	     long len)
	{
	ASN1_OBJECT *ret=NULL;
	const unsigned char *p;
	int i;
	for (i = 0, p = *pp + 1; i < len - 1; i++, p++)
		{
		if (*p == 0x80 && (!i || !(p[-1] & 0x80)))
			{
			ASN1err(ASN1_F_C2I_ASN1_OBJECT,ASN1_R_INVALID_OBJECT_ENCODING);
			return NULL;
			}
		}


	if ((a == NULL) || ((*a) == NULL) ||
		!((*a)->flags & ASN1_OBJECT_FLAG_DYNAMIC))
		{
		if ((ret=ASN1_OBJECT_new()) == NULL) return(NULL);
		}
	else	ret=(*a);

	p= *pp;
	if ((ret->data == NULL) || (ret->length < len))
		{
		if (ret->data != NULL) OPENSSL_free(ret->data);
		ret->data=(unsigned char *)OPENSSL_malloc(len ? (int)len : 1);
		ret->flags|=ASN1_OBJECT_FLAG_DYNAMIC_DATA;
		if (ret->data == NULL)
			{ i=ERR_R_MALLOC_FAILURE; goto err; }
		}
	memcpy(ret->data,p,(int)len);
	ret->length=(int)len;
	ret->sn=NULL;
	ret->ln=NULL;
	/* ret->flags=ASN1_OBJECT_FLAG_DYNAMIC; we know it is dynamic */
	p+=len;

	if (a != NULL) (*a)=ret;
	*pp=p;
	return(ret);
err:
	ASN1err(ASN1_F_C2I_ASN1_OBJECT,i);
	if ((ret != NULL) && ((a == NULL) || (*a != ret)))
		ASN1_OBJECT_free(ret);
	return(NULL);
	}

////////////////ASN1_OBJECT_new//////////////////////////ok

ASN1_OBJECT *ASN1_OBJECT_new(void)
	{
	ASN1_OBJECT *ret;
	ret=(ASN1_OBJECT *)OPENSSL_malloc(sizeof(ASN1_OBJECT));
	if (ret == NULL)
		{
		ASN1err(ASN1_F_ASN1_OBJECT_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->length=0;
	ret->data=NULL;
	ret->nid=0;
	ret->sn=NULL;
	ret->ln=NULL;
	ret->flags=ASN1_OBJECT_FLAG_DYNAMIC;
	return(ret);
	}

//////////ASN1_OBJECT_free/////////////////////ok

void ASN1_OBJECT_free(ASN1_OBJECT *a)
	{

	if (a == NULL) return;
	if (a->flags & ASN1_OBJECT_FLAG_DYNAMIC_STRINGS)
		{
#ifndef CONST_STRICT /* disable purely for compile-time strict const checking. Doing this on a "real" compile will cause memory leaks */
		if (a->sn != NULL) OPENSSL_free((void *)a->sn);
		if (a->ln != NULL) OPENSSL_free((void *)a->ln);
#endif
		a->sn=a->ln=NULL;
		}
	if (a->flags & ASN1_OBJECT_FLAG_DYNAMIC_DATA)
		{
		if (a->data != NULL) OPENSSL_free(a->data);
		a->data=NULL;
		a->length=0;
		}
	if (a->flags & ASN1_OBJECT_FLAG_DYNAMIC)
		OPENSSL_free(a);
	}


