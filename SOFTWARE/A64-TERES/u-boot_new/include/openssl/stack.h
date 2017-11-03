#ifndef HEADER_STACK_H
#define HEADER_STACK_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct stack_st
	{
	int num;
	char **data;
	int sorted;

	int num_alloc;
	int (*comp)(const char * const *, const char * const *);
	} STACK;

#define M_sk_num(sk)		((sk) ? (sk)->num:-1)
#define M_sk_value(sk,n)	((sk) ? (sk)->data[n] : NULL)

int sk_num(const STACK *);
char *sk_value(const STACK *, int);

char *sk_set(STACK *, int, char *);

STACK *sk_new(int (*cmp)(const char * const *, const char * const *));
STACK *sk_new_null(void);
void sk_free(STACK *);
void sk_pop_free(STACK *st, void (*func)(void *));
int sk_insert(STACK *sk,char *data,int where);
char *sk_delete(STACK *st,int loc);
char *sk_delete_ptr(STACK *st, char *p);
int sk_find(STACK *st,char *data);
int sk_find_ex(STACK *st,char *data);
int sk_push(STACK *st,char *data);
int sk_unshift(STACK *st,char *data);
char *sk_shift(STACK *st);
char *sk_pop(STACK *st);
void sk_zero(STACK *st);
int (*sk_set_cmp_func(STACK *sk, int (*c)(const char * const *,
			const char * const *)))
			(const char * const *, const char * const *);
STACK *sk_dup(STACK *st);
void sk_sort(STACK *st);
int sk_is_sorted(const STACK *st);

#ifdef  __cplusplus
}
#endif

#endif
