#include"asn1.h"
#include"err.h"
#include"myfunction.h"

////////////////////////////////////////////////////ok

ASN1_STRING *ASN1_STRING_type_new(int type)
{
	ASN1_STRING *ret;
	ret=(ASN1_STRING *)OPENSSL_malloc(sizeof(ASN1_STRING));
	if (ret == NULL)
		{
		ASN1err(ASN1_F_ASN1_STRING_TYPE_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->length=0;
	ret->type=type;
	ret->data=NULL;
	ret->flags=0;
	return(ret);
}
///////////////ASN1_STRING_set/////////////////////////////////ok


int ASN1_STRING_set(ASN1_STRING *str, const void *_data, int len)
	{
	unsigned char *c;
	const char *data=_data;
	if (len < 0)
		{
		if (data == NULL)
			return(0);
		else
			len=strlen(data);
		}
	if ((str->length < len) || (str->data == NULL))
		{
		c=str->data;
		if (c == NULL)
			str->data=OPENSSL_malloc(len+1);
		else
			str->data=OPENSSL_realloc(c,len+1);

		if (str->data == NULL)
			{
			ASN1err(ASN1_F_ASN1_STRING_SET,ERR_R_MALLOC_FAILURE);
			str->data=c;
			return(0);
			}
		}
	str->length=len;
	if (data != NULL)
		{
		memcpy(str->data,data,len);
		/* an allowance for strings :-) */
		str->data[len]='\0';
		}
	return(1);
	}

//////////////ASN1_STRING_free////////////////////////

void ASN1_STRING_free(ASN1_STRING *a)
	{

	if (a == NULL) return;
	if (a->data != NULL) OPENSSL_free(a->data);
	OPENSSL_free(a);
	}
