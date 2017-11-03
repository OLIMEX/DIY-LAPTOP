#include "evp.h"
#include "asn1t.h"
#include "x509.h"
#include "x509v3.h"

/////////////////X509_CINF_IT////////////////////////////////

ASN1_SEQUENCE_enc(X509_CINF, enc, 0) = {
//	ASN1_EXP_OPT(X509_CINF, version, ASN1_INTEGER, 0),
	ASN1_SIMPLE(X509_CINF, serialNumber, ASN1_INTEGER),//
	ASN1_SIMPLE(X509_CINF, signature, X509_ALGOR),//
//	ASN1_SIMPLE(X509_CINF, issuer, X509_NAME),
//	ASN1_SIMPLE(X509_CINF, validity, X509_VAL),
//	ASN1_SIMPLE(X509_CINF, subject, X509_NAME),
	ASN1_SIMPLE(X509_CINF, key, X509_PUBKEY),//
//	ASN1_IMP_OPT(X509_CINF, issuerUID, ASN1_BIT_STRING, 1),
//	ASN1_IMP_OPT(X509_CINF, subjectUID, ASN1_BIT_STRING, 2),
	ASN1_EXP_SEQUENCE_OF_OPT(X509_CINF, extensions, X509_EXTENSION, 3)//
} ASN1_SEQUENCE_END_enc(X509_CINF, X509_CINF)

//IMPLEMENT_ASN1_FUNCTIONS(X509_CINF)



static int x509_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it)
{
	X509 *ret = (X509 *)*pval;
		
	switch(operation) {

		case ASN1_OP_NEW_POST:
		ret->valid=0;
		ret->name = NULL;
		ret->ex_flags = 0;
		ret->ex_pathlen = -1;
		ret->skid = NULL;
		ret->akid = NULL;
		ret->aux = NULL;
		CRYPTO_new_ex_data(CRYPTO_EX_INDEX_X509, ret, &ret->ex_data);
		break;

		case ASN1_OP_FREE_POST:
		CRYPTO_free_ex_data(CRYPTO_EX_INDEX_X509, ret, &ret->ex_data);

		break;

	}

	return 1;

}

ASN1_SEQUENCE_ref(X509, x509_cb, CRYPTO_LOCK_X509) = {
	ASN1_SIMPLE(X509, cert_info, X509_CINF),
	ASN1_SIMPLE(X509, sig_alg, X509_ALGOR),
	ASN1_SIMPLE(X509, signature, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_ref(X509, X509)

IMPLEMENT_ASN1_FUNCTIONS(X509)

