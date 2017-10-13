#include"bio.h"
#include"errno.h"
#include"cryptlib.h"
#include"lhash.h"
#include"myfunction.h"

static int ex_data_check(void);

#define EX_IMPL(a) impl->cb_##a
#define IMPL_CHECK if(!impl) impl_check();
#define EX_DATA_CHECK(iffail) if(!ex_data && !ex_data_check()) {iffail}

typedef struct st_ex_class_item {
	int class_index;
	STACK_OF(CRYPTO_EX_DATA_FUNCS) *meth;
	int meth_num;
} EX_CLASS_ITEM;

typedef struct st_CRYPTO_EX_DATA_IMPL	CRYPTO_EX_DATA_IMPL;

static const CRYPTO_EX_DATA_IMPL *impl = NULL;
static LHASH *ex_data = NULL;

struct st_CRYPTO_EX_DATA_IMPL
	{

	int (*cb_new_ex_data)(int class_index, void *obj,
			CRYPTO_EX_DATA *ad);
	void (*cb_free_ex_data)(int class_index, void *obj,
			CRYPTO_EX_DATA *ad);
	};


///////////////////impl_default/////////////

static int int_new_ex_data(int class_index, void *obj,
		CRYPTO_EX_DATA *ad);

static void int_free_ex_data(int class_index, void *obj,
		CRYPTO_EX_DATA *ad);

static CRYPTO_EX_DATA_IMPL impl_default =
	{
	int_new_ex_data,
	int_free_ex_data
	};

///////////////ex_hash_cb//////////////////////ok

static unsigned long ex_hash_cb(const void *a_void)
	{

	return ((const EX_CLASS_ITEM *)a_void)->class_index;
	}

///////////////ex_cmp_cb/////////////////////////////ok

static int ex_cmp_cb(const void *a_void, const void *b_void)
	{

	return (((const EX_CLASS_ITEM *)a_void)->class_index -
		((const EX_CLASS_ITEM *)b_void)->class_index);
	}


///////////IMPL_CHECK///////////////////////ok

static void impl_check(void)
	{

	//CRYPTO_w_lock(CRYPTO_LOCK_EX_DATA);
	if(!impl)
		impl = &impl_default;
	//CRYPTO_w_unlock(CRYPTO_LOCK_EX_DATA);
	}

////////////////CRYPTO_new_ex_data/////////////////ok

int CRYPTO_new_ex_data(int class_index, void *obj, CRYPTO_EX_DATA *ad)
	{

	IMPL_CHECK
	return EX_IMPL(new_ex_data)(class_index, obj, ad);
	}

/////////////////CRYPTO_free_ex_data///////////////////////////ok

void CRYPTO_free_ex_data(int class_index, void *obj, CRYPTO_EX_DATA *ad)
	{

	IMPL_CHECK
	EX_IMPL(free_ex_data)(class_index, obj, ad);
	}

//////////bio_set////////////////////////ok

int BIO_set(BIO *bio, BIO_METHOD *method)
	{
	bio->method=method;
	bio->callback=NULL;
	bio->cb_arg=NULL;
	bio->init=0;
	bio->shutdown=1;
	bio->flags=0;
	bio->retry_reason=0;
	bio->num=0;
	bio->ptr=NULL;
	bio->prev_bio=NULL;
	bio->next_bio=NULL;
	bio->references=1;
	bio->num_read=0L;
	bio->num_write=0L;

	CRYPTO_new_ex_data(CRYPTO_EX_INDEX_BIO, bio, &bio->ex_data);
	if (method->create != NULL)
		if (!method->create(bio))
			{
			CRYPTO_free_ex_data(CRYPTO_EX_INDEX_BIO, bio,
					&bio->ex_data);
			return(0);
			}
	return(1);
	}


/////////////////BI0_new//////////////ok

BIO *BIO_new(BIO_METHOD *method)
{
	BIO *ret=NULL;

	ret=(BIO *)OPENSSL_malloc(sizeof(BIO));
	if (ret == NULL)
		{

		return(NULL);
		}
	if (!BIO_set(ret,method))
		{
		OPENSSL_free(ret);
		ret=NULL;
		}
	return(ret);
}


///////////////////def_get_class////////////////////////////////////////ok

static EX_CLASS_ITEM *def_get_class(int class_index)
	{
	EX_CLASS_ITEM d, *p, *gen;

	EX_DATA_CHECK(return NULL;)
	d.class_index = class_index;
	//CRYPTO_w_lock(CRYPTO_LOCK_EX_DATA);
	p = lh_retrieve(ex_data, &d);
	if(!p)
		{
		gen = OPENSSL_malloc(sizeof(EX_CLASS_ITEM));
		if(gen)
			{
			gen->class_index = class_index;
			gen->meth_num = 0;
			gen->meth = sk_CRYPTO_EX_DATA_FUNCS_new_null();
			if(!gen->meth)
				OPENSSL_free(gen);
			else
				{

				lh_insert(ex_data, gen);
				p = gen;
				}
			}
		}
	//CRYPTO_w_unlock(CRYPTO_LOCK_EX_DATA);
	if(!p)
		CRYPTOerr(CRYPTO_F_DEF_GET_CLASS,ERR_R_MALLOC_FAILURE);
	return p;
	}



///////////////////int_new_ex_data/////////////////////////////////////////ok

static int int_new_ex_data(int class_index, void *obj,
		CRYPTO_EX_DATA *ad)
	{
	int mx,i;
	CRYPTO_EX_DATA_FUNCS **storage = NULL;

	EX_CLASS_ITEM *item = def_get_class(class_index);

	if(!item)
		/* error is already set */
		return 0;
	ad->sk = NULL;
	//CRYPTO_r_lock(CRYPTO_LOCK_EX_DATA);
	mx = sk_CRYPTO_EX_DATA_FUNCS_num(item->meth);
	if(mx > 0)
		{
		storage = OPENSSL_malloc(mx * sizeof(CRYPTO_EX_DATA_FUNCS*));
		if(!storage)
			goto skip;
		for(i = 0; i < mx; i++)
			storage[i] = sk_CRYPTO_EX_DATA_FUNCS_value(item->meth,i);
		}
skip:
	//CRYPTO_r_unlock(CRYPTO_LOCK_EX_DATA);
	if((mx > 0) && !storage)
		{
		CRYPTOerr(CRYPTO_F_INT_NEW_EX_DATA,ERR_R_MALLOC_FAILURE);
		return 0;
		}
	for(i = 0; i < mx; i++)
		{
		if(storage[i] && storage[i]->new_func)
			{
				;
			}
		}
	if(storage)
		OPENSSL_free(storage);
	return 1;
	}




////////////ex_data_check//////////////////////////////////////////ok

static int ex_data_check(void)
	{
	int toret = 1;

	//CRYPTO_w_lock(CRYPTO_LOCK_EX_DATA);
	if(!ex_data && ((ex_data = lh_new(ex_hash_cb, ex_cmp_cb)) == NULL))
		toret = 0;
	//CRYPTO_w_unlock(CRYPTO_LOCK_EX_DATA);
	return toret;
	}

///////////////int_free_ex_data/////////////////////////////////////////ok

static void int_free_ex_data(int class_index, void *obj,
		CRYPTO_EX_DATA *ad)
	{
	int mx,i;
	EX_CLASS_ITEM *item;
	CRYPTO_EX_DATA_FUNCS **storage = NULL;

	if((item = def_get_class(class_index)) == NULL)
		return;
	//CRYPTO_r_lock(CRYPTO_LOCK_EX_DATA);
	mx = sk_CRYPTO_EX_DATA_FUNCS_num(item->meth);
	if(mx > 0)
		{
		storage = OPENSSL_malloc(mx * sizeof(CRYPTO_EX_DATA_FUNCS*));
		if(!storage)
			goto skip;
		for(i = 0; i < mx; i++)
			storage[i] = sk_CRYPTO_EX_DATA_FUNCS_value(item->meth,i);
		}
skip:
	//CRYPTO_r_unlock(CRYPTO_LOCK_EX_DATA);
	if((mx > 0) && !storage)
		{
		CRYPTOerr(CRYPTO_F_INT_FREE_EX_DATA,ERR_R_MALLOC_FAILURE);
		return;
		}
	for(i = 0; i < mx; i++)
		{
		if(storage[i] && storage[i]->free_func)
			{
				;
			}
		}
	if(storage)
		OPENSSL_free(storage);
	if(ad->sk)
		{
		sk_free(ad->sk);
		ad->sk=NULL;
		}
	}

void reset_BIO_reset(void)
{
	impl = NULL;
	ex_data = NULL;
}