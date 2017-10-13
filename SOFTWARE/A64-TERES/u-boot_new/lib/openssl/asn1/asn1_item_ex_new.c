#include "asn1.h"
#include "asn1t.h"
#include"err.h"
#include"objects.h"
#include"myfunction.h"

static int asn1_item_ex_combine_new(ASN1_VALUE **pval, const ASN1_ITEM *it,int combine);
static void asn1_template_clear(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt);
static void asn1_item_clear(ASN1_VALUE **pval, const ASN1_ITEM *it);
void asn1_primitive_clear(ASN1_VALUE **pval, const ASN1_ITEM *it);


/////////////////ASN1_item_ex_new/////////////////////////ok

int ASN1_item_ex_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	
	return asn1_item_ex_combine_new(pval, it, 0);
}

////////////ASN1_item_new//////////////////////////////////ok

ASN1_VALUE *ASN1_item_new(const ASN1_ITEM *it)
	{
	ASN1_VALUE *ret = NULL;
	
	if (ASN1_item_ex_new(&ret, it) > 0)
		return ret;
	return NULL;
	}
///////////////asn1_item_ex_combine_new//////////////////////////ok

static int asn1_item_ex_combine_new(ASN1_VALUE **pval, const ASN1_ITEM *it,
								int combine)
	{
	const ASN1_TEMPLATE *tt = NULL;
	const ASN1_COMPAT_FUNCS *cf;
	const ASN1_EXTERN_FUNCS *ef;
	const ASN1_AUX *aux = it->funcs;
	ASN1_aux_cb *asn1_cb;
	ASN1_VALUE **pseqval;
	int i;
	
	if (aux && aux->asn1_cb)
		asn1_cb = aux->asn1_cb;//x509_cb
	else
		asn1_cb = 0;

	if (!combine) *pval = NULL;

	switch(it->itype)
		{

		case ASN1_ITYPE_EXTERN:
		ef = it->funcs;
		if (ef && ef->asn1_ex_new)
			{
			if (!ef->asn1_ex_new(pval, it))
				goto memerr;
			}
		break;

		case ASN1_ITYPE_COMPAT:
		cf = it->funcs;
		if (cf && cf->asn1_new) {
			*pval = cf->asn1_new();
			if (!*pval)
				goto memerr;
		}
		break;

		case ASN1_ITYPE_PRIMITIVE:
		if (it->templates)
			{
			if (!ASN1_template_new(pval, it->templates))
				goto memerr;
			}
		else if (!ASN1_primitive_new(pval, it))
				goto memerr;
		break;

		case ASN1_ITYPE_MSTRING:
		if (!ASN1_primitive_new(pval, it))
				goto memerr;
		break;

		case ASN1_ITYPE_CHOICE:
		if (asn1_cb)
			{
			i = asn1_cb(ASN1_OP_NEW_PRE, pval, it);
			if (!i)
				goto auxerr;
			if (i==2)
				{

				return 1;
				}
			}
		if (!combine)
			{
			*pval = OPENSSL_malloc(it->size);
			if (!*pval)
				goto memerr;
			memset(*pval, 0, it->size);
			}
		//asn1_set_choice_selector(pval, -1, it);//samyang delete
		if (asn1_cb && !asn1_cb(ASN1_OP_NEW_POST, pval, it))
				goto auxerr;
		break;

		case ASN1_ITYPE_NDEF_SEQUENCE:
		case ASN1_ITYPE_SEQUENCE:
		if (asn1_cb)
			{
			i = asn1_cb(ASN1_OP_NEW_PRE, pval, it);
			if (!i)
				goto auxerr;
			if (i==2)
				{
				return 1;
				}
			}
		if (!combine)
			{
			*pval = OPENSSL_malloc(it->size);//接收结构体的地址
			if (!*pval)
				goto memerr;
			memset(*pval, 0, it->size);
			//asn1_do_lock(pval, 0, it);
			asn1_enc_init(pval, it);//?
			}
		for (i = 0, tt = it->templates; i < it->tcount; tt++, i++)//x509_seq_tt[]
			{
				pseqval = asn1_get_field_ptr(pval, tt);//返回"接收结构体"中的偏移地址	
			
				if (!ASN1_template_new(pseqval, tt))	//创建子item,根据x509_seq_tt[]中的xx_it
					goto memerr;
			
			}
		if (asn1_cb && !asn1_cb(ASN1_OP_NEW_POST, pval, it))
				goto auxerr;
	
		break;
	}

	return 1;

	memerr:
	ASN1err(ASN1_F_ASN1_ITEM_EX_COMBINE_NEW, ERR_R_MALLOC_FAILURE);
	return 0;

	auxerr:
	ASN1err(ASN1_F_ASN1_ITEM_EX_COMBINE_NEW, ASN1_R_AUX_ERROR);
	return 0;

	}

///////////////////ASN1_template_new///////////////////////////////ok

int ASN1_template_new(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
	{
	const ASN1_ITEM *it = ASN1_ITEM_ptr(tt->item);//
	
	int ret;

	if (tt->flags & ASN1_TFLG_OPTIONAL)
		{
		asn1_template_clear(pval, tt);
		return 1;
		}

	if (tt->flags & ASN1_TFLG_ADB_MASK)
		{
		*pval = NULL;
		return 1;
		}
	
	if (tt->flags & ASN1_TFLG_SK_MASK)
		{
		STACK_OF(ASN1_VALUE) *skval;
		skval = sk_ASN1_VALUE_new_null();
		if (!skval)
			{
			ASN1err(ASN1_F_ASN1_TEMPLATE_NEW, ERR_R_MALLOC_FAILURE);
			ret = 0;
			goto done;
			}
		*pval = (ASN1_VALUE *)skval;
		ret = 1;
		goto done;
		}
	
	ret = asn1_item_ex_combine_new(pval, it, tt->flags & ASN1_TFLG_COMBINE);//判断子item结构的类型，长度
	done:
	return ret;
	}

///////////////asn1_template_clear///////////////////////////ok

static void asn1_template_clear(ASN1_VALUE **pval, const ASN1_TEMPLATE *tt)
	{
	
	if (tt->flags & (ASN1_TFLG_ADB_MASK|ASN1_TFLG_SK_MASK)) 
		*pval = NULL;
	else
		asn1_item_clear(pval, ASN1_ITEM_ptr(tt->item));
	}


////////////////ASN1_primitive_new///////////////////////ok

int ASN1_primitive_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	ASN1_TYPE *typ;
	int utype;
	
	if (it && it->funcs)
		{
		const ASN1_PRIMITIVE_FUNCS *pf = it->funcs;
		if (pf->prim_new)
			return pf->prim_new(pval, it);
		}

	if (!it || (it->itype == ASN1_ITYPE_MSTRING))
		utype = -1;
	else
		utype = it->utype;
	switch(utype)
		{
		case V_ASN1_OBJECT:
		*pval = (ASN1_VALUE *)OBJ_nid2obj(NID_undef);
		return 1;

		case V_ASN1_BOOLEAN:
		if (it)
			*(ASN1_BOOLEAN *)pval = it->size;
		else
			*(ASN1_BOOLEAN *)pval = -1;
		return 1;

		case V_ASN1_NULL:
		*pval = (ASN1_VALUE *)1;
		return 1;

		case V_ASN1_ANY:
		typ = OPENSSL_malloc(sizeof(ASN1_TYPE));
		if (!typ)
			return 0;
		typ->value.ptr = NULL;
		typ->type = -1;
		*pval = (ASN1_VALUE *)typ;
		break;

		default:
		*pval = (ASN1_VALUE *)ASN1_STRING_type_new(utype);
		break;
		}
	if (*pval)
		return 1;
	return 0;
	}


///////////////asn1_item_clear///////////////////////////////ok

static void asn1_item_clear(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	const ASN1_EXTERN_FUNCS *ef;
	
	switch(it->itype)
		{

		case ASN1_ITYPE_EXTERN:
		ef = it->funcs;
		if (ef && ef->asn1_ex_clear)
			ef->asn1_ex_clear(pval, it);
		else *pval = NULL;
		break;


		case ASN1_ITYPE_PRIMITIVE:
		if (it->templates)
			asn1_template_clear(pval, it->templates);
		else
			asn1_primitive_clear(pval, it);
		break;

		case ASN1_ITYPE_MSTRING:
		asn1_primitive_clear(pval, it);
		break;

		case ASN1_ITYPE_COMPAT:
		case ASN1_ITYPE_CHOICE:
		case ASN1_ITYPE_SEQUENCE:
		case ASN1_ITYPE_NDEF_SEQUENCE:
		*pval = NULL;
		break;
		}
	}

////////////asn1_primitive_clear///////////////////////ok

void asn1_primitive_clear(ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	int utype;
	
	if (it && it->funcs)
		{
		const ASN1_PRIMITIVE_FUNCS *pf = it->funcs;
		if (pf->prim_clear)
			pf->prim_clear(pval, it);
		else
			*pval = NULL;
		return;
		}
	if (!it || (it->itype == ASN1_ITYPE_MSTRING))
		utype = -1;
	else
		utype = it->utype;
	if (utype == V_ASN1_BOOLEAN)
		*(ASN1_BOOLEAN *)pval = it->size;
	else *pval = NULL;
	}




