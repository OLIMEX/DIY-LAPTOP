/*
 * algif_rng: User-space interface for rng algorithms
 *
 * This file provides the user-space API for symmetric key ciphers.
 *
 * Copyright (c) 2013 Mintow <duanmintao@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <crypto/scatterwalk.h>
#include <crypto/skcipher.h>
#include <crypto/rng.h>
#include <crypto/if_alg.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/sock.h>

/* For debug */
#define RNG_DBG(fmt, arg...) pr_debug("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define RNG_ERR(fmt, arg...) pr_err("%s()%d - "fmt, __func__, __LINE__, ##arg)
#define RNG_EXIT()  		 RNG_DBG("%s \n", "Exit")
#define RNG_ENTER() 		 RNG_DBG("%s \n", "Enter ...")

struct rng_ctx {
	unsigned int len;
	struct crypto_rng *tfm;
};

static int rng_recvmsg(struct kiocb *unused, struct socket *sock,
			    struct msghdr *msg, size_t ignored, int flags)
{
	struct sock *sk = sock->sk;
	struct alg_sock *ask = alg_sk(sk);
	struct rng_ctx *ctx = ask->private;
	struct iovec *iov = msg->msg_iov;
	char *buf = NULL;
	int nbyte = 0;

	RNG_DBG("flags = %#x \n", flags);

	if ((iov->iov_base == NULL) || (iov->iov_len == 0)) {
		RNG_ERR("Invalid parameter: base %p, len %ld \n", iov->iov_base, iov->iov_len);
		return -EINVAL;
	}

	buf = kzalloc(iov->iov_len, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	lock_sock(sk);
	nbyte = crypto_rng_get_bytes(ctx->tfm, buf, iov->iov_len);
	release_sock(sk);
	
	if (nbyte > 0) {
		if (copy_to_user(iov->iov_base, buf, iov->iov_len))
			nbyte = -1;
	}

	kfree(buf);
	return nbyte;
}

static struct proto_ops algif_rng_ops = {
	.family		=	PF_ALG,

	.connect	=	sock_no_connect,
	.socketpair	=	sock_no_socketpair,
	.getname	=	sock_no_getname,
	.ioctl		=	sock_no_ioctl,
	.listen		=	sock_no_listen,
	.shutdown	=	sock_no_shutdown,
	.getsockopt	=	sock_no_getsockopt,
	.mmap		=	sock_no_mmap,
	.bind		=	sock_no_bind,
	.accept		=	sock_no_accept,
	.setsockopt	=	sock_no_setsockopt,

	.release	=	af_alg_release,
	.sendmsg	=	sock_no_sendmsg,
	.sendpage	=	sock_no_sendpage,
	.recvmsg	=	rng_recvmsg,
	.poll		=	sock_no_poll,
};

static void *rng_bind(const char *name, u32 type, u32 mask)
{
	RNG_DBG("name = %s, type = %d, mask = %#x \n", name, type, mask);
	return crypto_alloc_rng(name, type, mask);
}

static void rng_release(void *private)
{
	RNG_DBG("enter rng_release\n");
	crypto_free_rng(private);
}

static int rng_setkey(void *private, const u8 *key, unsigned int keylen)
{
	RNG_DBG("keylen = %d \n", keylen);
	return crypto_rng_reset(private, (u8 *)key, keylen);
}

static void rng_sock_destruct(struct sock *sk)
{
	struct alg_sock *ask = alg_sk(sk);
	struct rng_ctx *ctx = ask->private;

	RNG_ENTER();
	sock_kfree_s(sk, ctx, ctx->len);
	af_alg_release_parent(sk);
}

static int rng_accept_parent(void *private, struct sock *sk)
{
	struct rng_ctx *ctx;
	struct alg_sock *ask = alg_sk(sk);
	unsigned int len = sizeof(*ctx);

	RNG_ENTER();
	ctx = sock_kmalloc(sk, len, GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->len = len;
	ctx->tfm = (struct crypto_rng *)private;

	ask->private = ctx;
	sk->sk_destruct = rng_sock_destruct;
	return 0;
}

static const struct af_alg_type algif_type_rng = {
	.bind		=	rng_bind,
	.release	=	rng_release,
	.setkey		=	rng_setkey,
	.accept		=	rng_accept_parent,
	.ops		=	&algif_rng_ops,
	.name		=	"rng",
	.owner		=	THIS_MODULE
};

static int __init algif_rng_init(void)
{
	RNG_ENTER();
	return af_alg_register_type(&algif_type_rng);
}

static void __exit algif_rng_exit(void)
{
	int err = af_alg_unregister_type(&algif_type_rng);
	BUG_ON(err);
	RNG_DBG("err = %d \n", err);
}

module_init(algif_rng_init);
module_exit(algif_rng_exit);
MODULE_LICENSE("GPL");
