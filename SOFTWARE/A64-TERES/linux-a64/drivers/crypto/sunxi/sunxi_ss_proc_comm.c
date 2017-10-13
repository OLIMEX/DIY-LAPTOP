/*
 * The driver of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2014 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>

#include "sunxi_ss.h"
#include "sunxi_ss_proc.h"
#include "sunxi_ss_reg.h"

void ss_print_hex(char *_data, int _len, void *_addr)
{
	int i;

	pr_debug("-------------------- The valid len = %d ----------------------- ", _len);
	for (i=0; i<_len/8; i++) {
		pr_debug("0x%p: %02X %02X %02X %02X %02X %02X %02X %02X \n", i*8 + _addr,
			_data[i*8+0], _data[i*8+1], _data[i*8+2], _data[i*8+3],
			_data[i*8+4], _data[i*8+5], _data[i*8+6], _data[i*8+7]);
	}
	pr_debug("-------------------------------------------------------------- ");
}

int ss_sg_cnt(struct scatterlist *sg, int total)
{
	int cnt = 0;
	int nbyte = 0;
	struct scatterlist *cur = sg;
	struct scatterlist *prev = sg;

	while (cur != NULL) {
		cnt++;
		prev = cur;
		SS_DBG("cnt: %d, cur: %p, len: %d, is_last: %ld\n", cnt, cur,
				cur->length, sg_is_last(cur));
		nbyte += cur->length;
		if (nbyte >= total)
			break;

		cur = sg_next(cur);
	}

	if (!sg_is_last(prev))
		sg_mark_end(prev);
	return cnt;
}

int ss_aes_crypt(struct ablkcipher_request *req, int dir, int method, int mode)
{
	ss_aes_req_ctx_t *req_ctx = ablkcipher_request_ctx(req);

	SS_DBG("nbytes: %d, dec: %d, method: %d, mode: %d\n", req->nbytes, dir, method, mode);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend. \n");
		return -EAGAIN;
	}

	req_ctx->dir  = dir;
	req_ctx->type = method;
	req_ctx->mode = mode;
	req->base.flags |= SS_FLAG_AES;

	return ss_aes_one_req(ss_dev, req);
}

int ss_prng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen)
{
	return ss_rng_get_random(tfm, rdata, dlen, 0);
}

#ifdef SS_TRNG_ENABLE
int ss_trng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen)
{
	return ss_rng_get_random(tfm, rdata, dlen, 1);
}
#endif

#if defined(SS_SHA_SWAP_PRE_ENABLE) || defined(SS_SHA_SWAP_FINAL_ENABLE)
void ss_hash_swap(char *data, int len)
{
	int i;
	int temp = 0;
	int *cur = (int *)data;

	SS_DBG("Convert the byter-order of digest. len %d\n", len);
	for (i=0; i<len/4; i++, cur++) {
		temp = cpu_to_be32(*cur);
		*cur = temp;
	}
}
#endif

int ss_hash_blk_size(int type)
{
#if defined(SS_SHA384_ENABLE) || defined(SS_SHA512_ENABLE)
	if ((type == SS_METHOD_SHA384) || (type == SS_METHOD_SHA512))
		return SHA512_BLOCK_SIZE;
	else
#endif
		return SHA1_BLOCK_SIZE;
}

/* Prepare for padding in Hash. Final() will process the data. */
static void ss_hash_padding_data_prepare(ss_hash_ctx_t *ctx, char *tail, int len)
{
	if (len != 0)
		memcpy(ctx->pad, tail, len);

	SS_DBG("The padding data: \n");
	ss_print_hex(ctx->pad, len, NULL);
}

/* The tail data will be processed later. */
void ss_hash_padding_sg_prepare(struct scatterlist *last, int total)
{
	if (total%SHA1_BLOCK_SIZE != 0) {
		SS_DBG("sg len: %d, total: %d \n", sg_dma_len(last), total);
		WARN(sg_dma_len(last) < total%SHA1_BLOCK_SIZE, "sg len: %d, total: %d \n", sg_dma_len(last), total);
		sg_dma_len(last) = sg_dma_len(last) - total%SHA1_BLOCK_SIZE;
	}
	WARN_ON(sg_dma_len(last) > total);
}

int ss_hash_padding(ss_hash_ctx_t *ctx, int type)
{
	int n = ctx->cnt % ss_hash_blk_size(type);
	u8 *p = ctx->pad;
	int len_l = ctx->cnt << 3;  /* total len, in bits. */
	int len_h = ctx->cnt >> 29;
	int big_endian = type == SS_METHOD_MD5 ? 0 : 1;

	SS_DBG("type = %d, n = %d, ctx->cnt = %d\n", type, n, ctx->cnt);
	p[n] = 0x80;
	n++;

	if (n > (ss_hash_blk_size(type)-8)) {
		memset(p+n, 0, ss_hash_blk_size(type)*2 - n);
		p += ss_hash_blk_size(type)*2-8;
	}
	else {
		memset(p+n, 0, ss_hash_blk_size(type)-8-n);
		p += ss_hash_blk_size(type)-8;
	}

	if (big_endian == 1) {
		*(int *)p = swab32(len_h);
		*(int *)(p+4) = swab32(len_l);
	}
	else {
		*(int *)p = len_l;
		*(int *)(p+4) = len_h;
	}

	SS_DBG("After padding %ld: %02x %02x %02x %02x   %02x %02x %02x %02x\n",
			p + 8 - ctx->pad,
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

	return p + 8 - ctx->pad;
}

static int ss_hash_one_req(sunxi_ss_t *sss, struct ahash_request *req)
{
	int ret = 0;
	ss_aes_req_ctx_t *req_ctx = NULL;
	ss_hash_ctx_t *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));

	SS_ENTER();
	if (!req->src) {
		SS_ERR("Invalid sg: src = %p\n", req->src);
		return -EINVAL;
	}

	ss_dev_lock();

	req_ctx = ahash_request_ctx(req);
	req_ctx->dma_src.sg = req->src;

	ss_hash_padding_data_prepare(ctx, req->result, req->nbytes%ss_hash_blk_size(req_ctx->type));

	ret = ss_hash_start(ctx, req_ctx, req->nbytes);
	if (ret < 0)
		SS_ERR("ss_hash_start fail(%d)\n", ret);

	ss_dev_unlock();
	return ret;
}

int ss_hash_update(struct ahash_request *req)
{
	if (!req->nbytes) {
		SS_ERR("Invalid length: %d. \n", req->nbytes);
		return 0;
	}

	SS_DBG("Flags: %#x, len = %d \n", req->base.flags, req->nbytes);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend. \n");
		return -EAGAIN;
	}

	req->base.flags |= SS_FLAG_HASH;
	return ss_hash_one_req(ss_dev, req);
}

int ss_hash_final(struct ahash_request *req)
{
	int pad_len = 0;
	ss_aes_req_ctx_t *req_ctx = ahash_request_ctx(req);
	ss_hash_ctx_t *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(req));
	struct scatterlist last = {0}; /* make a sg struct for padding data. */

	if (req->result == NULL) {
		SS_ERR("Invalid result porinter. \n");
		return -EINVAL;
	}
	SS_DBG("Method: %d, cnt: %d\n", req_ctx->type, ctx->cnt);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend. \n");
		return -EAGAIN;
	}

	/* Process the padding data. */
	pad_len = ss_hash_padding(ctx, req_ctx->type);
	SS_DBG("Pad len: %d \n", pad_len);
	req_ctx->dma_src.sg = &last;
	sg_init_table(&last, 1);
	sg_set_buf(&last, ctx->pad, pad_len);
	SS_DBG("Padding data: \n");
	ss_print_hex((s8 *)ctx->pad, 128, ctx->pad);

	ss_dev_lock();
	ss_hash_start(ctx, req_ctx, pad_len);

	ss_sha_final();

	SS_DBG("Method: %d, cnt: %d\n", req_ctx->type, ctx->cnt);

	ss_check_sha_end();
	ss_md_get(req->result, ctx->md, ctx->md_size);
	ss_ctrl_stop();
	ss_dev_unlock();

#ifdef SS_SHA_SWAP_FINAL_ENABLE
	if (req_ctx->type != SS_METHOD_MD5)
		ss_hash_swap(req->result, ctx->md_size);
#endif

	return 0;
}

int ss_hash_finup(struct ahash_request *req)
{
	ss_hash_update(req);
	return ss_hash_final(req);
}

#ifdef SS_TRNG_POSTPROCESS_ENABLE
void ss_trng_postprocess(u8 *out, u32 outlen, u8 *in, u32 inlen)
{
	s32 i;
	u32 left = inlen;
	s8 result[SHA256_DIGEST_SIZE] = "";
	struct scatterlist sg = {0};
	struct crypto_hash *tfm = NULL;
	struct hash_desc desc = {0};

	tfm = crypto_alloc_hash("sha256", 0, CRYPTO_ALG_ASYNC);
	WARN(IS_ERR(tfm), "Failed to alloc sha256 tfm. %p\n", tfm);

	desc.tfm = tfm;
	desc.flags = 0;

	for (i=0; i<inlen/SHA256_BLOCK_SIZE; i++) {
		sg_init_one(&sg, in+i*SHA256_BLOCK_SIZE, SHA256_BLOCK_SIZE);
		if (crypto_hash_digest(&desc, &sg, SHA256_BLOCK_SIZE, result) != 0)
			SS_ERR("Failed to do %d SHA256(), inlen: %d\n", i, inlen);

		if (left < SHA256_DIGEST_SIZE) {
			memcpy(&out[i*SHA256_DIGEST_SIZE], result, inlen);
			break;
		}
		memcpy(&out[i*SHA256_DIGEST_SIZE], result, SHA256_DIGEST_SIZE);
		left -= SHA256_DIGEST_SIZE;
	}

	crypto_free_hash(tfm);
}
#endif

