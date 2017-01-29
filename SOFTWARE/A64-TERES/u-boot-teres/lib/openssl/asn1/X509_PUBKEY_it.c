#include "asn1.h"
#include "asn1t.h"
#include "x509v3.h"
#include "rsa.h"

static int pubkey_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it)
	{
	if (operation == ASN1_OP_FREE_POST)
		{
		X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
		EVP_PKEY_free(pubkey->pkey);
		}
	return 1;
	}

/////////////////X509_PUBKEY_it//////////////////////////////

ASN1_SEQUENCE_cb(X509_PUBKEY, pubkey_cb) = {
	ASN1_SIMPLE(X509_PUBKEY, algor, X509_ALGOR),
	ASN1_SIMPLE(X509_PUBKEY, public_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_cb(X509_PUBKEY, X509_PUBKEY)
	
IMPLEMENT_ASN1_FUNCTIONS(X509_PUBKEY)



