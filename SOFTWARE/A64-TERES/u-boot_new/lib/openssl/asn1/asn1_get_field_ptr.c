#include"asn1.h"
#include"asn1t.h"
#include"objects.h"
#include"err.h"


#define offset2ptr(addr, offset) (void *)(((char *) addr) + offset)

////////////////asn1_get_field_ptr///////////////////////////////////ok由tt中的offset计算要读取数据在"接收结构体"里面的偏移

ASN1_VALUE ** asn1_get_field_ptr(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
{
	ASN1_VALUE **pvaltmp;

	if (tt->flags & ASN1_TFLG_COMBINE)
		return pval;
	pvaltmp = offset2ptr(*pval, tt->offset);

	return pvaltmp;
}


////////////////asn1_get_enc_ptr/////////////////////////ok

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
////////////////asn1_enc_save//////////////////////////////ok

int asn1_enc_save(ASN1_VALUE **pval, const unsigned char *in, int inlen,
							 const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;

	enc = asn1_get_enc_ptr(pval, it);
	if (!enc)
		return 1;

	if (enc->enc)
		OPENSSL_free(enc->enc);
	enc->enc = OPENSSL_malloc(inlen);
	if (!enc->enc)
		return 0;
	memcpy(enc->enc, in, inlen);
	enc->len = inlen;
	enc->modified = 0;

	return 1;
	}

////////////////asn1_do_adb//////////////////////////////////////ok

const ASN1_TEMPLATE *asn1_do_adb(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt,int nullerr)
	{
	const ASN1_ADB *adb;
	const ASN1_ADB_TABLE *atbl;
	long selector = 0;
	ASN1_VALUE **sfld;
	int i;

	if (!(tt->flags & ASN1_TFLG_ADB_MASK))
		return tt;//??

	adb = ASN1_ADB_ptr(tt->item);

	sfld = offset2ptr(*pval, adb->offset);

	if (!sfld)
		{
		if (!adb->null_tt)
			goto err;
		return adb->null_tt;
		}

	if (tt->flags & ASN1_TFLG_ADB_OID)
		selector = OBJ_obj2nid((ASN1_OBJECT *)*sfld);
	else
		//selector = ASN1_INTEGER_get((ASN1_INTEGER *)*sfld);
		;


	for (atbl = adb->tbl, i = 0; i < adb->tblcount; i++, atbl++)
		if (atbl->value == selector)
			return &atbl->tt;


	if (!adb->default_tt)
		goto err;
	return adb->default_tt;

	err:
	if (nullerr)
		ASN1err(ASN1_F_ASN1_DO_ADB,
			ASN1_R_UNSUPPORTED_ANY_DEFINED_BY_TYPE);
	return NULL;
	}



///////////////asn1_enc_init////////////////////////////////////ok

void asn1_enc_init(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_ENCODING *enc;
	enc = asn1_get_enc_ptr(pval, it);
	if (enc)
		{
		enc->enc = NULL;
		enc->len = 0;
		enc->modified = 1;
		}
	}

