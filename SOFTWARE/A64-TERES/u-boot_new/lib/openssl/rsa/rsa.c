#include"rsa.h"
#include"err.h"
#include"engine.h"

//static RSA_METHOD rsa_pkcs1_eay_meth={		//--hgl--20140331--RW mem to const mem
const RSA_METHOD rsa_pkcs1_eay_meth={
	"Eric Young's PKCS#1 RSA",
	0, /* flags */
	NULL,
	0, /* rsa_sign */
	0, /* rsa_verify */
	NULL /* rsa_keygen */
	};
/////////////////////RSA_new////////////////////////////////////////ok

RSA *RSA_new(void)
	{
		
	RSA *r=RSA_new_method(NULL);
	
	return r;
	}

///////////////////RSA_new_method///////////////////////////////////////ok

RSA *RSA_new_method(ENGINE *engine)
	{
	RSA *ret;
	
	ret=(RSA *)OPENSSL_malloc(sizeof(RSA));
	if (ret == NULL)
		{
		RSAerr(RSA_F_RSA_NEW_METHOD,ERR_R_MALLOC_FAILURE);
		return NULL;
		}

	ret->meth = &rsa_pkcs1_eay_meth;

	ret->pad=0;
	ret->version=0;
	ret->n=NULL;
	ret->e=NULL;
	ret->d=NULL;
	ret->p=NULL;
	ret->q=NULL;
	ret->dmp1=NULL;
	ret->dmq1=NULL;
	ret->iqmp=NULL;
	ret->references=1;
	ret->_method_mod_n=NULL;
	ret->_method_mod_p=NULL;
	ret->_method_mod_q=NULL;
	ret->blinding=NULL;
	ret->mt_blinding=NULL;
	ret->bignum_data=NULL;
	ret->flags=ret->meth->flags & ~RSA_FLAG_NON_FIPS_ALLOW;
	if (!CRYPTO_new_ex_data(CRYPTO_EX_INDEX_RSA, ret, &ret->ex_data))
		{
		OPENSSL_free(ret);
		return(NULL);
		}

	if ((ret->meth->init != NULL) && !ret->meth->init(ret))
		{
		CRYPTO_free_ex_data(CRYPTO_EX_INDEX_RSA, ret, &ret->ex_data);
		OPENSSL_free(ret);
		ret=NULL;
		}
	return(ret);
	}



