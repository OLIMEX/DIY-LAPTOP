
#include"myfunction.h"
#include"lhash.h"




#undef MIN_NODES
#define MIN_NODES	16
#define UP_LOAD		(2*LH_LOAD_MULT)
#define DOWN_LOAD	(LH_LOAD_MULT)

static LHASH_NODE **getrn(LHASH *lh, const void *data, unsigned long *rhash);


////////////////getrn//////////////////////////ok

static LHASH_NODE **getrn(LHASH *lh, const void *data, unsigned long *rhash)
	{
	LHASH_NODE **ret,*n1;
	unsigned long hash,nn;
	LHASH_COMP_FN_TYPE cf;

	//DMSG_DEBUG("============getrn=================--1--\n");

	hash=(*(lh->hash))(data);
	lh->num_hash_calls++;
	*rhash=hash;

	//DMSG_DEBUG("============getrn=================--2--\n");

	nn=hash%lh->pmax;

	if (nn < lh->p)
		nn=hash%lh->num_alloc_nodes;

	cf=lh->comp;
	ret= &(lh->b[(int)nn]);

	//DMSG_DEBUG("============getrn=================--3--\n");

	for (n1= *ret; n1 != NULL; n1=n1->next)
		{
#ifndef OPENSSL_NO_HASH_COMP
		lh->num_hash_comps++;
		if (n1->hash != hash)
			{
			ret= &(n1->next);
			continue;
			}
#endif
		lh->num_comp_calls++;
		if(cf(n1->data,data) == 0)
			break;
		ret= &(n1->next);
		}

	//DMSG_DEBUG("============getrn=================--end--\n");

	return(ret);
	}

///////////lh_retrieve//////////////////////////ok

void *lh_retrieve(LHASH *lh, const void *data)
{
	unsigned long hash;
	LHASH_NODE **rn;
	void *ret;

	lh->error=0;
	rn=getrn(lh,data,&hash);

	if (*rn == NULL)
		{
		lh->num_retrieve_miss++;
		return(NULL);
		}
	else
		{
		ret= (*rn)->data;
		lh->num_retrieve++;
		}
	return(ret);
}


///////////////lh_insert//////////////////////////ok

void *lh_insert(LHASH *lh, void *data)
	{
	unsigned long hash;
	LHASH_NODE *nn,**rn;
	void *ret;

	lh->error=0;

	rn=getrn(lh,data,&hash);

	if (*rn == NULL)
		{
		if ((nn=(LHASH_NODE *)OPENSSL_malloc(sizeof(LHASH_NODE))) == NULL)
			{
			lh->error++;
			return(NULL);
			}
		nn->data=data;
		nn->next=NULL;
#ifndef OPENSSL_NO_HASH_COMP
		nn->hash=hash;
#endif
		*rn=nn;
		ret=NULL;
		lh->num_insert++;
		lh->num_items++;
		}
	else /* replace same key */
		{
		ret= (*rn)->data;
		(*rn)->data=data;
		lh->num_replace++;
		}
	return(ret);
	}


///////////lh_new/////////////////////////////ok

LHASH *lh_new(LHASH_HASH_FN_TYPE h, LHASH_COMP_FN_TYPE c)
	{
	LHASH *ret;
	int i;

	if ((ret=(LHASH *)OPENSSL_malloc(sizeof(LHASH))) == NULL)
		goto err0;
	if ((ret->b=(LHASH_NODE **)OPENSSL_malloc(sizeof(LHASH_NODE *)*MIN_NODES)) == NULL)
		goto err1;
	for (i=0; i<MIN_NODES; i++)
		ret->b[i]=NULL;
	ret->comp=((c == NULL)?(LHASH_COMP_FN_TYPE)strcmp:c);
	ret->hash=h;//((h == NULL)?(LHASH_HASH_FN_TYPE)lh_strhash:h);//samyang delete
	ret->num_nodes=MIN_NODES/2;
	ret->num_alloc_nodes=MIN_NODES;
	ret->p=0;
	ret->pmax=MIN_NODES/2;
	ret->up_load=UP_LOAD;
	ret->down_load=DOWN_LOAD;
	ret->num_items=0;

	ret->num_expands=0;
	ret->num_expand_reallocs=0;
	ret->num_contracts=0;
	ret->num_contract_reallocs=0;
	ret->num_hash_calls=0;
	ret->num_comp_calls=0;
	ret->num_insert=0;
	ret->num_replace=0;
	ret->num_delete=0;
	ret->num_no_delete=0;
	ret->num_retrieve=0;
	ret->num_retrieve_miss=0;
	ret->num_hash_comps=0;

	ret->error=0;
	return(ret);
err1:
	OPENSSL_free(ret);
err0:
	return(NULL);
	}

