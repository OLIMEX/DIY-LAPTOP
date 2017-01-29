#include "asn1.h"
#include "asn1t.h"
#include "x509v3.h"

/////////////////X509_EXTENSION_it///////////////////////////

ASN1_SEQUENCE(X509_EXTENSION) = {
	ASN1_SIMPLE(X509_EXTENSION, object, ASN1_OBJECT),
	ASN1_OPT(X509_EXTENSION, critical, ASN1_BOOLEAN),
	ASN1_SIMPLE(X509_EXTENSION, value, ASN1_OCTET_STRING)
} ASN1_SEQUENCE_END(X509_EXTENSION);
	
//////////////X509_ALGOR_it/////////////////////////////////

ASN1_SEQUENCE(X509_ALGOR) = {
	ASN1_SIMPLE(X509_ALGOR, algorithm, ASN1_OBJECT),
	ASN1_OPT(X509_ALGOR, parameter, ASN1_ANY)
} ASN1_SEQUENCE_END(X509_ALGOR)
