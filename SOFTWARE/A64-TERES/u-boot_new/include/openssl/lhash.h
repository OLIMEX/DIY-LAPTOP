

#ifndef HEADER_LHASH_H
#define HEADER_LHASH_H

#include "e_os2.h"

#ifndef OPENSSL_NO_BIO
#include "bio.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct lhash_node_st
	{
	void *data;
	struct lhash_node_st *next;
#ifndef OPENSSL_NO_HASH_COMP
	unsigned long hash;
#endif
	} LHASH_NODE;

typedef int (*LHASH_COMP_FN_TYPE)(const void *, const void *);
typedef unsigned long (*LHASH_HASH_FN_TYPE)(const void *);
typedef void (*LHASH_DOALL_FN_TYPE)(void *);
typedef void (*LHASH_DOALL_ARG_FN_TYPE)(void *, void *);

/* Macros for declaring and implementing type-safe wrappers for LHASH callbacks.
 * This way, callbacks can be provided to LHASH structures without function
 * pointer casting and the macro-defined callbacks provide per-variable casting
 * before deferring to the underlying type-specific callbacks. NB: It is
 * possible to place a "static" in front of both the DECLARE and IMPLEMENT
 * macros if the functions are strictly internal. */

/* First: "hash" functions */
#define DECLARE_LHASH_HASH_FN(f_name,o_type) \
	unsigned long f_name##_LHASH_HASH(const void *);
#define IMPLEMENT_LHASH_HASH_FN(f_name,o_type) \
	unsigned long f_name##_LHASH_HASH(const void *arg) { \
		o_type a = (o_type)arg; \
		return f_name(a); }
#define LHASH_HASH_FN(f_name) f_name##_LHASH_HASH

/* Second: "compare" functions */
#define DECLARE_LHASH_COMP_FN(f_name,o_type) \
	int f_name##_LHASH_COMP(const void *, const void *);
#define IMPLEMENT_LHASH_COMP_FN(f_name,o_type) \
	int f_name##_LHASH_COMP(const void *arg1, const void *arg2) { \
		o_type a = (o_type)arg1; \
		o_type b = (o_type)arg2; \
		return f_name(a,b); }
#define LHASH_COMP_FN(f_name) f_name##_LHASH_COMP

/* Third: "doall" functions */
#define DECLARE_LHASH_DOALL_FN(f_name,o_type) \
	void f_name##_LHASH_DOALL(void *);
#define IMPLEMENT_LHASH_DOALL_FN(f_name,o_type) \
	void f_name##_LHASH_DOALL(void *arg) { \
		o_type a = (o_type)arg; \
		f_name(a); }
#define LHASH_DOALL_FN(f_name) f_name##_LHASH_DOALL

/* Fourth: "doall_arg" functions */
#define DECLARE_LHASH_DOALL_ARG_FN(f_name,o_type,a_type) \
	void f_name##_LHASH_DOALL_ARG(void *, void *);
#define IMPLEMENT_LHASH_DOALL_ARG_FN(f_name,o_type,a_type) \
	void f_name##_LHASH_DOALL_ARG(void *arg1, void *arg2) { \
		o_type a = (o_type)arg1; \
		a_type b = (a_type)arg2; \
		f_name(a,b); }
#define LHASH_DOALL_ARG_FN(f_name) f_name##_LHASH_DOALL_ARG

typedef struct lhash_st
	{
	LHASH_NODE **b;
	LHASH_COMP_FN_TYPE comp;
	LHASH_HASH_FN_TYPE hash;
	unsigned int num_nodes;
	unsigned int num_alloc_nodes;
	unsigned int p;
	unsigned int pmax;
	unsigned long up_load; /* load times 256 */
	unsigned long down_load; /* load times 256 */
	unsigned long num_items;

	unsigned long num_expands;
	unsigned long num_expand_reallocs;
	unsigned long num_contracts;
	unsigned long num_contract_reallocs;
	unsigned long num_hash_calls;
	unsigned long num_comp_calls;
	unsigned long num_insert;
	unsigned long num_replace;
	unsigned long num_delete;
	unsigned long num_no_delete;
	unsigned long num_retrieve;
	unsigned long num_retrieve_miss;
	unsigned long num_hash_comps;

	int error;
	} LHASH;

#define LH_LOAD_MULT	256

/* Indicates a malloc() error in the last call, this is only bad
 * in lh_insert(). */
#define lh_error(lh)	((lh)->error)

LHASH *lh_new(LHASH_HASH_FN_TYPE h, LHASH_COMP_FN_TYPE c);
void lh_free(LHASH *lh);
void *lh_insert(LHASH *lh, void *data);
void *lh_delete(LHASH *lh, const void *data);
void *lh_retrieve(LHASH *lh, const void *data);
void lh_doall(LHASH *lh, LHASH_DOALL_FN_TYPE func);
void lh_doall_arg(LHASH *lh, LHASH_DOALL_ARG_FN_TYPE func, void *arg);
unsigned long lh_strhash(const char *c);
unsigned long lh_num_items(const LHASH *lh);

#ifdef OPENSSL_NO_FP_API
void lh_stats(const LHASH *lh, FILE *out);
void lh_node_stats(const LHASH *lh, FILE *out);
void lh_node_usage_stats(const LHASH *lh, FILE *out);
#endif

#ifndef OPENSSL_NO_BIO
void lh_stats_bio(const LHASH *lh, BIO *out);
void lh_node_stats_bio(const LHASH *lh, BIO *out);
void lh_node_usage_stats_bio(const LHASH *lh, BIO *out);
#endif
#ifdef  __cplusplus
}
#endif

#endif

