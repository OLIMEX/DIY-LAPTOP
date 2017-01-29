#include"asn1.h"
#include"obj_dat.h"
#include"lhash.h"
#include"objects.h"
#include"err.h"
#include"cryptlib.h"
#include"limits.h"

#define ADDED_NID	3
#define ADDED_DATA	0

//#define OBJerr(f,r)  ERR_PUT_error(ERR_LIB_OBJ,(f),(r),__FILE__,__LINE__)

typedef struct added_obj_st
{
	int type;
	ASN1_OBJECT *obj;
} ADDED_OBJ;

static LHASH *added=NULL;

void reset_OBJ_nid2ln_reset(void)
{
	added=NULL;
}
/////////////OBJ_nid2ln/////////////////////////ok

const char *OBJ_nid2ln(int n)
{
	ADDED_OBJ ad,*adp;
	ASN1_OBJECT ob;

	if ((n >= 0) && (n < NUM_NID))
	{
		if ((n != NID_undef) && (nid_objs[n].nid == NID_undef))
			{
			return(NULL);
			}
		return(nid_objs[n].ln);
	}
	else if (added == NULL)
		return(NULL);
	else
	{
		ad.type=ADDED_NID;
		ad.obj= &ob;
		ob.nid=n;
		adp=(ADDED_OBJ *)lh_retrieve(added,&ad);
		if (adp != NULL)
			return(adp->obj->ln);
		else
		{
			return(NULL);
		}
	}
}
///////////////OBJ_bsearch/////////////////ok

const char *OBJ_bsearch(const char *key, const char *base, int num, int size,
	int (*cmp)(const void *, const void *))
{

	return OBJ_bsearch_ex(key, base, num, size, cmp, 0);
}

//////////////////////////////////////////////////////////////////ok

const char *OBJ_bsearch_ex(const char *key, const char *base, int num,
	int size, int (*cmp)(const void *, const void *), int flags)
{
	int l,h,i=0,c=0;
	const char *p = NULL;

	if (num == 0) return(NULL);
	l=0;
	h=num;
	while (l < h)
		{
		i=(l+h)/2;
		p= &(base[i*size]);
		c=(*cmp)(key,p);
		if (c < 0)
			h=i;
		else if (c > 0)
			l=i+1;
		else
			break;
}
//#ifdef CHARSET_EBCDIC			//###samyang  modity
/* THIS IS A KLUDGE - Because the *_obj is sorted in ASCII order, and
 * I don't have perl (yet), we revert to a *LINEAR* search
 * when the object wasn't found in the binary search.
 */
	if (c != 0)
		{
		for (i=0; i<num; ++i)
			{
			p= &(base[i*size]);
			c = (*cmp)(key,p);
			if (c == 0 || (c < 0 && (flags & OBJ_BSEARCH_VALUE_ON_NOMATCH)))
				return p;
			}
		}
//#endif		////###samyang  modity
	if (c != 0 && !(flags & OBJ_BSEARCH_VALUE_ON_NOMATCH))
		p =NULL;//&(base[78*size]);
	else if (c == 0 && (flags & OBJ_BSEARCH_FIRST_VALUE_ON_MATCH))
		{
		while(i > 0 && (*cmp)(key,&(base[(i-1)*size])) == 0)
			i--;
		p = &(base[i*size]);
		}
	return(p);
	}

//////////////////obj_cmp//////////////////ok

static int obj_cmp(const void *ap, const void *bp)
{
	int j;
	const ASN1_OBJECT *a= *(ASN1_OBJECT * const *)ap;
	const ASN1_OBJECT *b= *(ASN1_OBJECT * const *)bp;

	j=(a->length - b->length);
        if (j) return(j);
	return(memcmp(a->data,b->data,a->length));
  }
///////////////////OBJ_obj2nid//////////////////////ok

int OBJ_obj2nid(const ASN1_OBJECT *a)
	{
	ASN1_OBJECT **op;
	ADDED_OBJ ad,*adp;

	if (a == NULL)
		return(NID_undef);
	if (a->nid != 0)
		return(a->nid);

	if (added != NULL)
		{
		ad.type=ADDED_DATA;
		ad.obj=(ASN1_OBJECT *)a; /* XXX: ugly but harmless */
		adp=(ADDED_OBJ *)lh_retrieve(added,&ad);
		if (adp != NULL) return (adp->obj->nid);
		}
	op=(ASN1_OBJECT **)OBJ_bsearch((const char *)&a,(const char *)obj_objs,
		NUM_OBJ, sizeof(ASN1_OBJECT *),obj_cmp);
	if (op == NULL)
		return(NID_undef);
	return((*op)->nid);
	}


///////////////OBJ_nid2obj////////////////////////////////ok

ASN1_OBJECT *OBJ_nid2obj(int n)
{
	ADDED_OBJ ad,*adp;
	ASN1_OBJECT ob;

	if ((n >= 0) && (n < NUM_NID))
	{
		if ((n != NID_undef) && (nid_objs[n].nid == NID_undef))
		{

			return(NULL);
		}
		return((ASN1_OBJECT *)&(nid_objs[n]));
	}
	else if (added == NULL)
		return(NULL);
	else
	{
		ad.type=ADDED_NID;
		ad.obj= &ob;
		ob.nid=n;
		adp=(ADDED_OBJ *)lh_retrieve(added,&ad);
		if (adp != NULL)
			return(adp->obj);
		else
		{
			return(NULL);
		}
	}
}

const char *OBJ_nid2sn(int n)
{
	ADDED_OBJ ad,*adp;
	ASN1_OBJECT ob;

	if ((n >= 0) && (n < NUM_NID))
	{
		if ((n != NID_undef) && (nid_objs[n].nid == NID_undef))
			{
			OBJerr(OBJ_F_OBJ_NID2SN,OBJ_R_UNKNOWN_NID);
			return(NULL);
			}
		return(nid_objs[n].sn);
	}
	else if (added == NULL)
		return(NULL);
	else
	{
		ad.type=ADDED_NID;
		ad.obj= &ob;
		ob.nid=n;
		adp=lh_retrieve(added,&ad);
		if (adp != NULL)
			return(adp->obj->sn);
		else
			{
				return(NULL);
			}
	}
}


int OBJ_obj2txt(char *buf, int buf_len, const ASN1_OBJECT *a, int no_name)
{
//	int i,n=0,len,nid, first, use_bn;
//	BIGNUM *bl;
//	unsigned long l;
//	const unsigned char *p;
//	char tbuf[DECIMAL_SIZE(i)+DECIMAL_SIZE(l)+2];
//
//	if ((a == NULL) || (a->data == NULL)) {
//		buf[0]='\0';
//		return(0);
//	}
//
//	if (!no_name && (nid=OBJ_obj2nid(a)) != NID_undef)
//		{
//		const char *s;
//		s=OBJ_nid2ln(nid);
//		if (s == NULL)
//			s=OBJ_nid2sn(nid);
//		if (s)
//			{
//			if (buf)
//				strncpy(buf,s,buf_len);
//			n=strlen(s);
//			return n;
//			}
//		}
//
//	len=a->length;
//	p=a->data;
//
//	first = 1;
//	bl = NULL;
//
//	while (len > 0)
//		{
//		l=0;
//		use_bn = 0;
//		for (;;)
//			{
//			unsigned char c = *p++;
//			len--;
//			if ((len == 0) && (c & 0x80))
//				goto err;
//			if (use_bn)
//				{
//				if (!BN_add_word(bl, c & 0x7f))
//					goto err;
//				}
//			else
//				l |= c  & 0x7f;
//			if (!(c & 0x80))
//				break;
//			if (!use_bn && (l > (ULONG_MAX >> 7L)))
//				{
//				if (!bl && !(bl = BN_new()))
//					goto err;
//				if (!BN_set_word(bl, l))
//					goto err;
//				use_bn = 1;
//				}
//			if (use_bn)
//				{
//				if (!BN_lshift(bl, bl, 7))
//					goto err;
//				}
//			else
//				l<<=7L;
//			}
//
//		if (first)
//			{
//			first = 0;
//			if (l >= 80)
//				{
//				i = 2;
//				if (use_bn)
//					{
//					if (!BN_sub_word(bl, 80))
//						goto err;
//					}
//				else
//					l -= 80;
//				}
//			else
//				{
//				i=(int)(l/40);
//				l-=(long)(i*40);
//				}
//			if (buf && (buf_len > 0))
//				{
//				*buf++ = i + '0';
//				buf_len--;
//				}
//			n++;
//			}
//
//		if (use_bn)
//			{
//			char *bndec;
//			bndec = BN_bn2dec(bl);
//			if (!bndec)
//				goto err;
//			i = strlen(bndec);
//			if (buf)
//				{
//				if (buf_len > 0)
//					{
//					*buf++ = '.';
//					buf_len--;
//					}
//				strncpy(buf,bndec,buf_len);
//				if (i > buf_len)
//					{
//					buf += buf_len;
//					buf_len = 0;
//					}
//				else
//					{
//					buf+=i;
//					buf_len-=i;
//					}
//				}
//			n++;
//			n += i;
//			OPENSSL_free(bndec);
//			}
//		else
//			{
//			BIO_snprintf(tbuf,sizeof tbuf,".%lu",l);
//			i=strlen(tbuf);
//			if (buf && (buf_len > 0))
//				{
//				strncpy(buf,tbuf,buf_len);
//				if (i > buf_len)
//					{
//					buf += buf_len;
//					buf_len = 0;
//					}
//				else
//					{
//					buf+=i;
//					buf_len-=i;
//					}
//				}
//			n+=i;
//			l=0;
//			}
//		}
//
//	if (bl)
//		BN_free(bl);
//	return n;
//
//	err:
//	if (bl)
//		BN_free(bl);
	return -1;
}

int OBJ_obj2name(char *dst_buf, int buf_len, const ASN1_OBJECT *a)
{
	if(buf_len < a->length)
	{
		printf("OBJ_obj2name err: not enough buffer to store name\n");

		return -1;
	}
	memcpy(dst_buf, a->data, a->length);

	return a->length;
}
