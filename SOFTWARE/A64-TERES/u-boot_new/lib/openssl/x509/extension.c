#include"x509.h"
#include"stack.h"
#include"x509v3.h"
#include"err.h"
#include"myfunction.h"

#define sk_X509_EXTENSION_value(st, i) SKM_sk_value(X509_EXTENSION, (st), (i))
#define sk_X509_EXTENSION_num(st) SKM_sk_num(X509_EXTENSION, (st))

////////////sk_num//////////////////////////////////ok

int sk_num(const STACK *st)
{

	if(st == NULL) return -1;
	return st->num;
}
//////////sk_value//////////////////////////////ok

char *sk_value(const STACK *st, int i)
{

	if(!st || (i < 0) || (i >= st->num)) return NULL;
	return st->data[i];
}
/////////X509v3_get_ext_count//////////////////////////////ok


