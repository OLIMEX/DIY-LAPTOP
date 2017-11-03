#include "cryptlib.h"
#include "asn1t.h"
#include "bn.h"


#define BN_SENSITIVE	1

static int bn_new(ASN1_VALUE **pval, const ASN1_ITEM *it);
static void bn_free(ASN1_VALUE **pval, const ASN1_ITEM *it);

static int bn_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it);
static int bn_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len, int utype, char *free_cont, const ASN1_ITEM *it);

static ASN1_PRIMITIVE_FUNCS bignum_pf = {
	NULL, 0,
	bn_new,
	bn_free,
	0,
	bn_c2i,
	bn_i2c
};

////////////////////////BIGNUM_it//////////////////////////////////////////////

ASN1_ITEM_start(BIGNUM)
	ASN1_ITYPE_PRIMITIVE, V_ASN1_INTEGER, NULL, 0, &bignum_pf, 0, "BIGNUM"
ASN1_ITEM_end(BIGNUM)

//////////////////bn_new//////////////////////////////////ok

static int bn_new(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
		
	*pval = (ASN1_VALUE *)BN_new();
	if(*pval) return 1;
	else return 0;
}

////////////////bn_free//////////////////////////////////

static void bn_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
		;
}

//////////////////bn_i2c/////////////////////////////////////////////////////////////ok

static int bn_i2c(ASN1_VALUE **pval, unsigned char *cont, int *putype, const ASN1_ITEM *it)
{
	BIGNUM *bn;
	int pad;
		
	if(!*pval) return -1;
	bn = (BIGNUM *)*pval;
	/* If MSB set in an octet we need a padding byte */
	if(BN_num_bits(bn) & 0x7) pad = 0;
	else pad = 1;
	if(cont) {
		if(pad) *cont++ = 0;
		BN_bn2bin(bn, cont);
	}
	return pad + BN_num_bytes(bn);
}

////////////////////bn_c2i/////////////////////////////////////////////ok

static int bn_c2i(ASN1_VALUE **pval, const unsigned char *cont, int len,
		  int utype, char *free_cont, const ASN1_ITEM *it)
{
	BIGNUM *bn;
		
	if(!*pval) bn_new(pval, it);
	bn  = (BIGNUM *)*pval;
	if(!BN_bin2bn(cont, len, bn)) {
		bn_free(pval, it);
		return 0;
	}
	return 1;
}


