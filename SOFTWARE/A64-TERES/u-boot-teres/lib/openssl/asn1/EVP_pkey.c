#include"x509.h"
#include"evp.h"
#include"rsa.h"

//////////EVP_PKEY_type//////////////////////ok

int EVP_PKEY_type(int type)
{

	switch (type)
		{
	case EVP_PKEY_RSA:
	case EVP_PKEY_RSA2:
		return(EVP_PKEY_RSA);
	case EVP_PKEY_DSA:
	case EVP_PKEY_DSA1:
	case EVP_PKEY_DSA2:
	case EVP_PKEY_DSA3:
	case EVP_PKEY_DSA4:
		return(EVP_PKEY_DSA);
	case EVP_PKEY_DH:
		return(EVP_PKEY_DH);
	case EVP_PKEY_EC:
		return(EVP_PKEY_EC);
	default:
		return(NID_undef);
		}
}

///////////////EVP_PKEY_new/////////////////////////////ok

EVP_PKEY *EVP_PKEY_new(void)
{
	EVP_PKEY *ret;

	ret=(EVP_PKEY *)OPENSSL_malloc(sizeof(EVP_PKEY));
	if (ret == NULL)
		{
		//EVPerr(EVP_F_EVP_PKEY_NEW,ERR_R_MALLOC_FAILURE);//samyang modify
		return(NULL);
		}
	ret->type=EVP_PKEY_NONE;
	ret->references=1;
	ret->pkey.ptr=NULL;
	ret->attributes=NULL;
	ret->save_parameters=1;
	return(ret);
}
///////////////X509_PUBKEY_get////////////////////////ok

EVP_PKEY *X509_PUBKEY_get(X509_PUBKEY *key)
{

	EVP_PKEY *ret=NULL;
	long j;
	int type;
	const unsigned char *p;

	if (key == NULL) goto err;

	if (key->pkey != NULL)
		{
		CRYPTO_add(&key->pkey->references, 1, CRYPTO_LOCK_EVP_PKEY);
		return(key->pkey);
		}

	if (key->public_key == NULL) goto err;

	type=OBJ_obj2nid(key->algor->algorithm);
	if ((ret = EVP_PKEY_new()) == NULL)
		{
		//X509err(X509_F_X509_PUBKEY_GET, ERR_R_MALLOC_FAILURE);//samyang modify
		goto err;
		}
	ret->type = EVP_PKEY_type(type);



#if !defined(OPENSSL_NO_DSA) || !defined(OPENSSL_NO_ECDSA)
	//a=key->algor;
#endif

	p=key->public_key->data;
        j=key->public_key->length;
        if (!d2i_PublicKey(type, &ret, &p, (long)j))
		{
		//X509err(X509_F_X509_PUBKEY_GET, X509_R_ERR_ASN1_LIB);//samyang modify
		goto err;
		}

	key->pkey = ret;
	CRYPTO_add(&ret->references, 1, CRYPTO_LOCK_EVP_PKEY);
	return(ret);
err:
	if (ret != NULL)
		EVP_PKEY_free(ret);
	return(NULL);

}

//////////////////X509_get_pubkey/////////////ok

EVP_PKEY *X509_get_pubkey(X509 *x)
{

	if ((x == NULL) || (x->cert_info == NULL))
		return(NULL);
	return(X509_PUBKEY_get(x->cert_info->key));
}



////////////EVP_PKEY_free/////////////////////ok

void EVP_PKEY_free(EVP_PKEY *x)
{
	int i;
	if (x == NULL) return;
	i=CRYPTO_add(&x->references,-1,CRYPTO_LOCK_EVP_PKEY);
	if (i > 0) return;

	}
