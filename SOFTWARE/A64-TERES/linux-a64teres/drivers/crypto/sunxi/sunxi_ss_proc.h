/*
 * Declare the function interface of SUNXI SS process.
 *
 * Copyright (C) 2014 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _SUNXI_SECURITY_SYSTEM_PROC_H_
#define _SUNXI_SECURITY_SYSTEM_PROC_H_

#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/algapi.h>
#include <linux/scatterlist.h>

/* Inner functions declaration, defined in vx/sunxi_ss_proc.c */

int ss_aes_key_valid(struct crypto_ablkcipher *tfm, int len);
int ss_aes_one_req(sunxi_ss_t *sss, struct ablkcipher_request *req);

int ss_rng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen, u32 trng);

int ss_hash_start(ss_hash_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx, int len);

irqreturn_t sunxi_ss_irq_handler(int irq, void *dev_id);

/* defined in sunxi_ss_proc_comm.c */

void ss_print_hex(char *_data, int _len, void *_addr);
int ss_sg_cnt(struct scatterlist *sg, int total);

int ss_prng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen);
#ifdef SS_TRNG_ENABLE
int ss_trng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen);
#endif
#ifdef SS_TRNG_POSTPROCESS_ENABLE
void ss_trng_postprocess(u8 *out, u32 outlen, u8 *in, u32 inlen);
#endif

int ss_aes_crypt(struct ablkcipher_request *req, int dir, int method, int mode);

void ss_hash_swap(char *data, int len);
int ss_hash_blk_size(int type);
int ss_hash_padding(ss_hash_ctx_t *ctx, int type);
void ss_hash_padding_sg_prepare(struct scatterlist *last, int total);
int ss_hash_update(struct ahash_request *req);
int ss_hash_final(struct ahash_request *req);
int ss_hash_finup(struct ahash_request *req);

#endif /* end of _SUNXI_SECURITY_SYSTEM_PROC_H_ */

