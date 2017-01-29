#include"cryptlib.h"
#include"bn_lcl.h"
#include"limits.h"
#include"assert.h"
#include"bn.h"
//#define BN_ULONG	unsigned long	//samyang add  depend on bn.h
//#define BN_BITS2	32

struct bn_blinding_st
	{
	BIGNUM *A;
	BIGNUM *Ai;
	BIGNUM *e;
	BIGNUM *mod; /* just a reference */
	unsigned long thread_id; /* added in OpenSSL 0.9.6j and 0.9.7b;
				  * used only by crypto/rsa/rsa_eay.c, rsa_lib.c */
	unsigned int  counter;
	unsigned long flags;
	BN_MONT_CTX *m_ctx;
	int (*bn_mod_exp)(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
			  const BIGNUM *m, BN_CTX *ctx,
			  BN_MONT_CTX *m_ctx);
	};

typedef struct bn_blinding_st BN_BLINDING;


///////////////BN_new/////////////////////////////////ok

BIGNUM *BN_new(void)
	{
	BIGNUM *ret;
	if ((ret=(BIGNUM *)OPENSSL_malloc(sizeof(BIGNUM))) == NULL)
		{
		BNerr(BN_F_BN_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->flags=BN_FLG_MALLOCED;
	ret->top=0;
	ret->neg=0;
	ret->dmax=0;
	ret->d=NULL;
	bn_check_top(ret);
	return(ret);
	}

////////////////bn_expand_internal//////////////////////////////ok

static BN_ULONG *bn_expand_internal(const BIGNUM *b, int words)
	{
	BN_ULONG *A,*a = NULL;
	const BN_ULONG *B;
	int i;
	bn_check_top(b);

	if (words > (INT_MAX/(4*BN_BITS2)))
		{
		BNerr(BN_F_BN_EXPAND_INTERNAL,BN_R_BIGNUM_TOO_LONG);
		return NULL;
		}
	if (BN_get_flags(b,BN_FLG_STATIC_DATA))
		{
		BNerr(BN_F_BN_EXPAND_INTERNAL,BN_R_EXPAND_ON_STATIC_BIGNUM_DATA);
		return(NULL);
		}
	a=A=(BN_ULONG *)OPENSSL_malloc(sizeof(BN_ULONG)*words);
	if (A == NULL)
		{
		BNerr(BN_F_BN_EXPAND_INTERNAL,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
#if 1
	B=b->d;

	if (B != NULL)
		{
		for (i=b->top>>2; i>0; i--,A+=4,B+=4)
			{

			BN_ULONG a0,a1,a2,a3;
			a0=B[0]; a1=B[1]; a2=B[2]; a3=B[3];
			A[0]=a0; A[1]=a1; A[2]=a2; A[3]=a3;
			}
		switch (b->top&3)
			{
		case 3:	A[2]=B[2];
		case 2:	A[1]=B[1];
		case 1:	A[0]=B[0];
		case 0:
			;
			}
		}

#else
	memset(A,0,sizeof(BN_ULONG)*words);
	memcpy(A,b->d,sizeof(b->d[0])*b->top);
#endif

	return(a);
	}

//////////////bn_expand2///////////////////////////////ok

BIGNUM *bn_expand2(BIGNUM *b, int words)
	{
	bn_check_top(b);

	if (words > b->dmax)
		{
		BN_ULONG *a = bn_expand_internal(b, words);
		if(!a) return NULL;
		if(b->d) OPENSSL_free(b->d);
		b->d=a;
		b->dmax=words;
		}
	bn_check_top(b);
	return b;
	}

////////////BN_num_bits_word//////////////////////////ok

int BN_num_bits_word(BN_ULONG l)
	{
	static const char bits[256]={
		0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		};


#ifdef SIXTY_FOUR_BIT
	if (l & 0xffffffff00000000LL)
		{
		if (l & 0xffff000000000000LL)
			{
			if (l & 0xff00000000000000LL)
				{
				return(bits[(int)(l>>56)]+56);
				}
			else	return(bits[(int)(l>>48)]+48);
			}
		else
			{
			if (l & 0x0000ff0000000000LL)
				{
				return(bits[(int)(l>>40)]+40);
				}
			else	return(bits[(int)(l>>32)]+32);
			}
		}
	else
#endif

		{
#if defined(THIRTY_TWO_BIT) || defined(SIXTY_FOUR_BIT) || defined(SIXTY_FOUR_BIT_LONG)
		if (l & 0xffff0000L)
			{
			if (l & 0xff000000L)
				return(bits[(int)(l>>24L)]+24);
			else	return(bits[(int)(l>>16L)]+16);
			}
		else
#endif
			{
#if defined(SIXTEEN_BIT) || defined(THIRTY_TWO_BIT) || defined(SIXTY_FOUR_BIT) || defined(SIXTY_FOUR_BIT_LONG)
			if (l & 0xff00L)
				return(bits[(int)(l>>8)]+8);
			else
#endif
				return(bits[(int)(l   )]  );
			}
		}
	}

////////////BN_num_bits///////////////////////////ok

int BN_num_bits(const BIGNUM *a)
{
	int i = a->top - 1;
	bn_check_top(a);

	if (BN_is_zero(a)) return 0;
	return ((i*BN_BITS2) + BN_num_bits_word(a->d[i]));
}

BN_ULONG BN_div_word(BIGNUM *a, BN_ULONG w)
	{
	BN_ULONG ret = 0;
	int i, j;

	bn_check_top(a);
	w &= BN_MASK2;

	if (!w)
		/* actually this an error (division by zero) */
		return (BN_ULONG)-1;
	if (a->top == 0)
		return 0;

	/* normalize input (so bn_div_words doesn't complain) */
	j = BN_BITS2 - BN_num_bits_word(w);
	w <<= j;
	if (!BN_lshift(a, a, j))
		return (BN_ULONG)-1;

	for (i=a->top-1; i>=0; i--)
		{
		BN_ULONG l,d;

		l=a->d[i];
		d=bn_div_words(ret,l,w);
		ret=(l-((d*w)&BN_MASK2))&BN_MASK2;
		a->d[i]=d;
		}
	if ((a->top > 0) && (a->d[a->top-1] == 0))
		a->top--;
	ret >>= j;
	bn_check_top(a);
	return(ret);
	}


int BN_add_word(BIGNUM *a, BN_ULONG w)
{
	BN_ULONG l;
	int i;

	bn_check_top(a);
	w &= BN_MASK2;

	/* degenerate case: w is zero */
	if (!w) return 1;
	/* degenerate case: a is zero */
	if(BN_is_zero(a)) return BN_set_word(a, w);
	/* handle 'a' when negative */
	if (a->neg)
		{
		a->neg=0;
		i=BN_sub_word(a,w);
		if (!BN_is_zero(a))
			a->neg=!(a->neg);
		return(i);
		}
	for (i=0;w!=0 && i<a->top;i++)
		{
		a->d[i] = l = (a->d[i]+w)&BN_MASK2;
		w = (w>l)?1:0;
		}
	if (w && i==a->top)
		{
		if (bn_wexpand(a,a->top+1) == NULL) return 0;
		a->top++;
		a->d[i]=w;
		}
	bn_check_top(a);
	return(1);
}

int BN_sub_word(BIGNUM *a, BN_ULONG w)
	{
	int i;

	bn_check_top(a);
	w &= BN_MASK2;

	/* degenerate case: w is zero */
	if (!w) return 1;
	/* degenerate case: a is zero */
	if(BN_is_zero(a))
		{
		i = BN_set_word(a,w);
		if (i != 0)
			BN_set_negative(a, 1);
		return i;
		}
	/* handle 'a' when negative */
	if (a->neg)
		{
		a->neg=0;
		i=BN_add_word(a,w);
		a->neg=1;
		return(i);
		}

	if ((a->top == 1) && (a->d[0] < w))
		{
		a->d[0]=w-a->d[0];
		a->neg=1;
		return(1);
		}
	i=0;
	for (;;)
		{
		if (a->d[i] >= w)
			{
			a->d[i]-=w;
			break;
			}
		else
			{
			a->d[i]=(a->d[i]-w)&BN_MASK2;
			i++;
			w=1;
			}
		}
	if ((a->d[i] == 0) && (i == (a->top-1)))
		a->top--;
	bn_check_top(a);
	return(1);
	}
