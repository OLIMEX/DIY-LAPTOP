#include"asn1.h"
#include"x509.h"


///////////X509_get_serialNumber///////////////////ok

ASN1_INTEGER *X509_get_serialNumber(X509 *a)
	{
	return(a->cert_info->serialNumber);
	}

