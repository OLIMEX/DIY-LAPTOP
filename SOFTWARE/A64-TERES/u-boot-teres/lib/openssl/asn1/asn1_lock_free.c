#include "asn1.h"
#include "asn1t.h"
#include "myfunction.h"

#define offset2ptr(addr, offset) (void *)(((char *) addr) + offset)


///////////////////////asn1_get_enc_ptr//////////////////////ok

static ASN1_ENCODING *asn1_get_enc_ptr(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	const ASN1_AUX *aux;
	
	if (!pval || !*pval)
		return NULL;
	aux = it->funcs;
	if (!aux || !(aux->flags & ASN1_AFLG_ENCODING))
		return NULL;
	return offset2ptr(*pval, aux->enc_offset);
	}

//////////////////asn1_enc_free//////////////////////////////ok

void asn1_enc_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	
	enc = asn1_get_enc_ptr(pval, it);
	if (enc)
		{
		if (enc->enc)
			OPENSSL_free(enc->enc);
		enc->enc = NULL;
		enc->len = 0;
		enc->modified = 1;
		}
	}


///////////////asn1_enc_restore/////////////////////////////////////////ok

int asn1_enc_restore(int *len, unsigned char **out, ASN1_VALUE **pval,
							const ASN1_ITEM *it)
{
	ASN1_ENCODING *enc;
	
	enc = asn1_get_enc_ptr(pval, it);
	if (!enc || enc->modified)
		return 0;
	if (out)
		{
		memcpy(*out, enc->enc, enc->len);
		*out += enc->len;
		}
	if (len)
		*len = enc->len;
	return 1;
}

