/* asn1t.h */
/* Written by Dr Stephen N Henson (steve@openssl.org) for the OpenSSL
 * project 2006.
 */
/* ====================================================================
 * Copyright (c) 2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* Internal ASN1 structures and functions: not for application use */

/* ASN1 print context structure */
#ifndef  __LOCLL_H__
#define  __LOCLL_H__

struct asn1_pctx_st
	{
	unsigned long flags;
	unsigned long nm_flags;
	unsigned long cert_flags;
	unsigned long oid_flags;
	unsigned long str_flags;
	} /* ASN1_PCTX */;

/* ASN1 public key method structure */

struct evp_pkey_asn1_method_st
	{
	int pkey_id;
	int pkey_base_id;
	unsigned long pkey_flags;

	char *pem_str;
	char *info;

	int (*pub_decode)(int *pk, int *pub);
	int (*pub_encode)(int *pub, const int *pk);
	int (*pub_cmp)(const int *a, const int *b);
	int (*pub_print)(BIO *out, const int *pkey, int indent,
							int *pctx);

	int (*priv_decode)(int *pk, int *p8inf);
	int (*priv_encode)(int *p8, const int *pk);
	int (*priv_print)(BIO *out, const int *pkey, int indent,
							int *pctx);

	int (*pkey_size)(const int *pk);
	int (*pkey_bits)(const int *pk);

	int (*param_decode)(int *pkey,
				const unsigned char **pder, int derlen);
	int (*param_encode)(const int *pkey, unsigned char **pder);
	int (*param_missing)(const int *pk);
	int (*param_copy)(int *to, const int *from);
	int (*param_cmp)(const int *a, const int *b);
	int (*param_print)(BIO *out, const int *pkey, int indent,
							int *pctx);
	int (*sig_print)(BIO *out,
			 const int *sigalg, const int *sig,
					 int indent, int *pctx);


	void (*pkey_free)(int *pkey);
	int (*pkey_ctrl)(int *pkey, int op, long arg1, void *arg2);

	/* Legacy functions for old PEM */

	int (*old_priv_decode)(int *pkey,
				const unsigned char **pder, int derlen);
	int (*old_priv_encode)(const int *pkey, unsigned char **pder);
	/* Custom ASN1 signature verification */
	int (*item_verify)(int *ctx, const int *it, void *asn,
				int *a, int *sig,
				int *pkey);
	int (*item_sign)(int *ctx, const int *it, void *asn,
				int *alg1, int *alg2,
				int *sig);

	} /* EVP_PKEY_ASN1_METHOD */;

/* Method to handle CRL access.
 * In general a CRL could be very large (several Mb) and can consume large
 * amounts of resources if stored in memory by multiple processes.
 * This method allows general CRL operations to be redirected to more
 * efficient callbacks: for example a CRL entry database.
 */

#define X509_CRL_METHOD_DYNAMIC		1


#endif
