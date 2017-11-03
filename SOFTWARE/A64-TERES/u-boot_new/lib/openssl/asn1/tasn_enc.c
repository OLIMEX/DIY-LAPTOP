#include "cryptlib.h"
#include "asn1.h"
#include "asn1t.h"
#include "objects.h"

static int asn1_i2d_ex_primitive(ASN1_VALUE **pval, unsigned char **out,
					const ASN1_ITEM *it,
					int tag, int aclass);
static int asn1_template_ex_i2d(ASN1_VALUE **pval, unsigned char **out,
					const ASN1_TEMPLATE *tt,
					int tag, int aclass);
static int asn1_item_flags_i2d(ASN1_VALUE *val, unsigned char **out,
					const ASN1_ITEM *it, int flags);


//////////////////ASN1_item_i2d//////////////////////////////////////////ok

int ASN1_item_i2d(ASN1_VALUE *val, unsigned char **out, const ASN1_ITEM *it)
	{

	return asn1_item_flags_i2d(val, out, it, 0);
	}

/////////////////asn1_item_flags_i2d/////////////////////////////////////////ok

static int asn1_item_flags_i2d(ASN1_VALUE *val, unsigned char **out,
					const ASN1_ITEM *it, int flags)
	{
	/*if (out && !*out)
		{
		unsigned char *p, *buf;
		int len;
		len = ASN1_item_ex_i2d(&val, NULL, it, -1, flags);
		if (len <= 0)
			return len;
		buf = OPENSSL_malloc(len);
		if (!buf)
			return -1;
		p = buf;
		ASN1_item_ex_i2d(&val, &p, it, -1, flags);
		*out = buf;
		return len;
		}
*/
	return ASN1_item_ex_i2d(&val, out, it, -1, flags);
	}

///////////////// ASN1_item_ex_i2d//////////////////////////////////ok

int ASN1_item_ex_i2d(ASN1_VALUE **pval, unsigned char **out,
			const ASN1_ITEM *it, int tag, int aclass)
	{
	const ASN1_TEMPLATE *tt = NULL;
	unsigned char *p = NULL;
	int i, seqcontlen, seqlen, ndef = 1;
	const ASN1_COMPAT_FUNCS *cf;
	const ASN1_EXTERN_FUNCS *ef;
	const ASN1_AUX *aux = it->funcs;
	ASN1_aux_cb *asn1_cb = 0;

	if ((it->itype != ASN1_ITYPE_PRIMITIVE) && !*pval)
		return 0;

	if (aux && aux->asn1_cb)
		 asn1_cb = aux->asn1_cb;

	switch(it->itype)
		{

		case ASN1_ITYPE_PRIMITIVE:
		if (it->templates)
			return asn1_template_ex_i2d(pval, out, it->templates,
								tag, aclass);
		return asn1_i2d_ex_primitive(pval, out, it, tag, aclass);
		break;

		case ASN1_ITYPE_MSTRING:
		return asn1_i2d_ex_primitive(pval, out, it, -1, aclass);

		case ASN1_ITYPE_CHOICE:
		if (asn1_cb && !asn1_cb(ASN1_OP_I2D_PRE, pval, it))
				return 0;


		if (asn1_cb && !asn1_cb(ASN1_OP_I2D_POST, pval, it))
				return 0;
		break;

		case ASN1_ITYPE_EXTERN:
		/* If new style i2d it does all the work */
		ef = it->funcs;
		return ef->asn1_ex_i2d(pval, out, it, tag, aclass);

		case ASN1_ITYPE_COMPAT:
		/* old style hackery... */
		cf = it->funcs;
		if (out)
			p = *out;
		i = cf->asn1_i2d(*pval, out);

		if (out && (tag != -1))
			*p = aclass | tag | (*p & V_ASN1_CONSTRUCTED);
		return i;

		case ASN1_ITYPE_NDEF_SEQUENCE:

		if (aclass & ASN1_TFLG_NDEF) ndef = 2;


		case ASN1_ITYPE_SEQUENCE:
		i = asn1_enc_restore(&seqcontlen, out, pval, it);
		/* An error occurred */
		if (i < 0)
			return 0;
		/* We have a valid cached encoding... */
		if (i > 0)
			return seqcontlen;
		/* Otherwise carry on */
		seqcontlen = 0;
		/* If no IMPLICIT tagging set to SEQUENCE, UNIVERSAL */
		if (tag == -1)
			{
			tag = V_ASN1_SEQUENCE;
			/* Retain any other flags in aclass */
			aclass = (aclass & ~ASN1_TFLG_TAG_CLASS)
					| V_ASN1_UNIVERSAL;
			}
		if (asn1_cb && !asn1_cb(ASN1_OP_I2D_PRE, pval, it))
				return 0;
		/* First work out sequence content length */
		for (i = 0, tt = it->templates; i < it->tcount; tt++, i++)
			{
			const ASN1_TEMPLATE *seqtt;
			ASN1_VALUE **pseqval;
			seqtt = asn1_do_adb(pval, tt, 1);
			if (!seqtt)
				return 0;
			pseqval = asn1_get_field_ptr(pval, seqtt);
			/* FIXME: check for errors in enhanced version */
			seqcontlen += asn1_template_ex_i2d(pseqval, NULL, seqtt,
								-1, aclass);
			}

		seqlen = ASN1_object_size(ndef, seqcontlen, tag);
		if (!out)
			return seqlen;

		ASN1_put_object(out, ndef, seqcontlen, tag, aclass);
		for (i = 0, tt = it->templates; i < it->tcount; tt++, i++)
			{
			const ASN1_TEMPLATE *seqtt;
			ASN1_VALUE **pseqval;
			seqtt = asn1_do_adb(pval, tt, 1);
			if (!seqtt)
				return 0;
			pseqval = asn1_get_field_ptr(pval, seqtt);

			asn1_template_ex_i2d(pseqval, out, seqtt, -1, aclass);
			}
		//if (ndef == 2)
		//	ASN1_put_eoc(out);//samyang delete
		if (asn1_cb  && !asn1_cb(ASN1_OP_I2D_POST, pval, it))
				return 0;
		return seqlen;

		default:
		return 0;

		}
	return 0;
	}


/////////////////asn1_template_ex_i2d/////////////////////////////////////ok

static int asn1_template_ex_i2d(ASN1_VALUE **pval, unsigned char **out,
				const ASN1_TEMPLATE *tt, int tag, int iclass)
	{
	int i, ret, flags, ttag, tclass, ndef;
	flags = tt->flags;

	if (flags & ASN1_TFLG_TAG_MASK)
		{
		if (tag != -1)
			return -1;
		ttag = tt->tag;
		tclass = flags & ASN1_TFLG_TAG_CLASS;
		}
	else if (tag != -1)
		{
		ttag = tag;
		tclass = iclass & ASN1_TFLG_TAG_CLASS;
		}
	else
		{
		ttag = -1;
		tclass = 0;
		}

	iclass &= ~ASN1_TFLG_TAG_CLASS;

	if ((flags & ASN1_TFLG_NDEF) && (iclass & ASN1_TFLG_NDEF))
		ndef = 2;
	else ndef = 1;

	if (flags & ASN1_TFLG_SK_MASK)
		{
		/* SET OF, SEQUENCE OF */
		STACK_OF(ASN1_VALUE) *sk = (STACK_OF(ASN1_VALUE) *)*pval;
		int isset, sktag, skaclass;
		int skcontlen, sklen;
		ASN1_VALUE *skitem;

		if (!*pval)
			return 0;

		if (flags & ASN1_TFLG_SET_OF)
			{
			isset = 1;
			/* 2 means we reorder */
			if (flags & ASN1_TFLG_SEQUENCE_OF)
				isset = 2;
			}
		else isset = 0;
		if ((ttag != -1) && !(flags & ASN1_TFLG_EXPTAG))
			{
			sktag = ttag;
			skaclass = tclass;
			}
		else
			{
			skaclass = V_ASN1_UNIVERSAL;
			if (isset)
				sktag = V_ASN1_SET;
			else sktag = V_ASN1_SEQUENCE;
			}

		/* Determine total length of items */
		skcontlen = 0;
		for (i = 0; i < sk_ASN1_VALUE_num(sk); i++)
			{
			skitem = sk_ASN1_VALUE_value(sk, i);
			skcontlen += ASN1_item_ex_i2d(&skitem, NULL,
						ASN1_ITEM_ptr(tt->item),
							-1, iclass);
			}
		sklen = ASN1_object_size(ndef, skcontlen, sktag);
		/* If EXPLICIT need length of surrounding tag */
		if (flags & ASN1_TFLG_EXPTAG)
			ret = ASN1_object_size(ndef, sklen, ttag);
		else ret = sklen;

		if (!out)
			return ret;


		if (flags & ASN1_TFLG_EXPTAG)
			ASN1_put_object(out, ndef, sklen, ttag, tclass);

		ASN1_put_object(out, ndef, skcontlen, sktag, skaclass);



		return ret;
		}

	if (flags & ASN1_TFLG_EXPTAG)
		{

		i = ASN1_item_ex_i2d(pval, NULL, ASN1_ITEM_ptr(tt->item),
								-1, iclass);
		if (!i)
			return 0;

		ret = ASN1_object_size(ndef, i, ttag);
		if (out)
			{

			ASN1_put_object(out, ndef, i, ttag, tclass);
			ASN1_item_ex_i2d(pval, out, ASN1_ITEM_ptr(tt->item),
								-1, iclass);
			//if (ndef == 2)
				//ASN1_put_eoc(out);//samyang delete
			}
		return ret;
		}

	return ASN1_item_ex_i2d(pval, out, ASN1_ITEM_ptr(tt->item),
						ttag, tclass | iclass);

}


////////////////////asn1_i2d_ex_primitive/////////////////////////////////ok

static int asn1_i2d_ex_primitive(ASN1_VALUE **pval, unsigned char **out,
				const ASN1_ITEM *it, int tag, int aclass)
	{
	int len;
	int utype;
	int usetag;
	int ndef = 0;

	utype = it->utype;

	len = asn1_ex_i2c(pval, NULL, &utype, it);


	if ((utype == V_ASN1_SEQUENCE) || (utype == V_ASN1_SET) ||
	   (utype == V_ASN1_OTHER))
		usetag = 0;
	else usetag = 1;

	if (len == -1)
		return 0;


	if (len == -2)
		{
		ndef = 2;
		len = 0;
		}


	if (tag == -1) tag = utype;


	if (out)
		{
		if (usetag)
			ASN1_put_object(out, ndef, len, tag, aclass);
		asn1_ex_i2c(pval, *out, &utype, it);
		//if (ndef)
			//ASN1_put_eoc(out);//samyang delete
		//else
			*out += len;
		}

	if (usetag)
		return ASN1_object_size(ndef, len, tag);
	return len;
	}

////////////////asn1_ex_i2c//////////////////////////////////ok

int asn1_ex_i2c(ASN1_VALUE **pval, unsigned char *cout, int *putype,
				const ASN1_ITEM *it)
	{
	ASN1_BOOLEAN *tbool = NULL;
	ASN1_STRING *strtmp;
	ASN1_OBJECT *otmp;
	int utype;
	unsigned char *cont=NULL, c;
	int len=0;
	const ASN1_PRIMITIVE_FUNCS *pf;
	pf = it->funcs;

	if (pf && pf->prim_i2c)
		return pf->prim_i2c(pval, cout, putype, it);

	if ((it->itype != ASN1_ITYPE_PRIMITIVE)
		|| (it->utype != V_ASN1_BOOLEAN))
		{
		if (!*pval) return -1;
		}

	if (it->itype == ASN1_ITYPE_MSTRING)
		{

		strtmp = (ASN1_STRING *)*pval;
		utype = strtmp->type;
		*putype = utype;
		}
	else if (it->utype == V_ASN1_ANY)
		{
		ASN1_TYPE *typ;
		typ = (ASN1_TYPE *)*pval;
		utype = typ->type;
		*putype = utype;
		pval = &typ->value.asn1_value;
		}
	else utype = *putype;

	switch(utype)
		{
		case V_ASN1_OBJECT:
		otmp = (ASN1_OBJECT *)*pval;
		cont = otmp->data;
		len = otmp->length;
		break;

		case V_ASN1_NULL:
		cont = NULL;
		len = 0;
		break;

		case V_ASN1_BOOLEAN:
		tbool = (ASN1_BOOLEAN *)pval;
		if (*tbool == -1)
			return -1;
		if (it->utype != V_ASN1_ANY)
			{
			if (*tbool && (it->size > 0))
				return -1;
			if (!*tbool && !it->size)
				return -1;
			}
		c = (unsigned char)*tbool;
		cont = &c;
		len = 1;
		break;

		case V_ASN1_BIT_STRING:
			;

		break;

		case V_ASN1_INTEGER:
		case V_ASN1_NEG_INTEGER:
		case V_ASN1_ENUMERATED:
		case V_ASN1_NEG_ENUMERATED:
					;
		//return i2c_ASN1_INTEGER((ASN1_INTEGER *)*pval,
							//cout ? &cout : NULL);
		break;

		case V_ASN1_OCTET_STRING:
		case V_ASN1_NUMERICSTRING:
		case V_ASN1_PRINTABLESTRING:
		case V_ASN1_T61STRING:
		case V_ASN1_VIDEOTEXSTRING:
		case V_ASN1_IA5STRING:
		case V_ASN1_UTCTIME:
		case V_ASN1_GENERALIZEDTIME:
		case V_ASN1_GRAPHICSTRING:
		case V_ASN1_VISIBLESTRING:
		case V_ASN1_GENERALSTRING:
		case V_ASN1_UNIVERSALSTRING:
		case V_ASN1_BMPSTRING:
		case V_ASN1_UTF8STRING:
		case V_ASN1_SEQUENCE:
		case V_ASN1_SET:
		default:
		/* All based on ASN1_STRING and handled the same */
		strtmp = (ASN1_STRING *)*pval;
		/* Special handling for NDEF */
		if ((it->size == ASN1_TFLG_NDEF)
			&& (strtmp->flags & ASN1_STRING_FLAG_NDEF))
			{
			if (cout)
				{
				strtmp->data = cout;
				strtmp->length = 0;
				}
			/* Special return code */
			return -2;
			}
		cont = strtmp->data;
		len = strtmp->length;

		break;

		}
	if (cout && len)
		memcpy(cout, cont, len);
	return len;
	}
