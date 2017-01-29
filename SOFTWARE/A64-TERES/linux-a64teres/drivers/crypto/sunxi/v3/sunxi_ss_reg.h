/*
 * The register macro of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2014 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _SUNXI_SECURITY_SYSTEM_REG_H_
#define _SUNXI_SECURITY_SYSTEM_REG_H_

//#include <mach/platform.h>

/* CE: Crypto Engine, start using CE from sun8iw7/sun8iw9 */
#define CE_REG_TSK			0x00
#define CE_REG_CTL			0x04
#define CE_REG_ICR			0x08
#define CE_REG_ISR			0x0C
#define CE_REG_TLR			0x10
#define CE_REG_TSR			0x14
#define CE_REG_ERR			0x18
#define CE_REG_CSS			0x1C
#define CE_REG_CDS			0x20
#define CE_REG_CSA			0x24
#define CE_REG_CDA			0x28
#define CE_REG_TPR			0x2C

#define CE_CHAN_INT_ENABLE		1

#define CE_CHAN_PENDING			1

#define CE_REG_TSR_BUSY			1
#define CE_REG_TSR_IDLE			0
#define CE_REG_TSR_BUSY_SHIFT	0
#define CE_REG_TSR_BUSY_MASK	(0x1 << CE_REG_TSR_BUSY_SHIFT)

#define CE_REG_ESR_ERR_UNSUPPORT	0
#define CE_REG_ESR_ERR_LEN			1
#define CE_REG_ESR_CHAN_SHIFT		4
#define CE_REG_ESR_CHAN_MASK(flow)	(0xF << (CE_REG_ESR_CHAN_SHIFT*flow))

#define CE_REG_CSS_OFFSET_SHIFT		16
#define CE_REG_CDS_OFFSET_SHIFT		16

/* About the common control word */

#define CE_TASK_INT_ENABLE			1
#define CE_COMM_CTL_TASK_INT_SHIFT	31
#define CE_COMM_CTL_TASK_INT_MASK	(0x1 << CE_COMM_CTL_TASK_INT_SHIFT)

#define CE_CBC_MAC_LEN_SHIFT		17

#define CE_HASH_IV_DEFAULT			0
#define CE_HASH_IV_INPUT			1
#define CE_COMM_CTL_IV_MODE_SHIFT	16

#define CE_HMAC_SHA1_LAST			BIT(15)

#define SS_DIR_ENCRYPT				0
#define SS_DIR_DECRYPT				1
#define CE_COMM_CTL_OP_DIR_SHIFT	8

#define SS_METHOD_AES				0
#define SS_METHOD_DES				1
#define SS_METHOD_3DES				2
#define SS_METHOD_MD5				16
#define SS_METHOD_SHA1				17
#define SS_METHOD_SHA224			18
#define SS_METHOD_SHA256			19
#define SS_METHOD_SHA384			20
#define SS_METHOD_SHA512			21
#define SS_METHOD_HMAC_SHA1			22
#define SS_METHOD_HMAC_SHA256		23
#define SS_METHOD_RSA				32
#define SS_METHOD_DH				SS_METHOD_RSA
#define SS_METHOD_TRNG				48
#define SS_METHOD_PRNG				49
#define SS_METHOD_ECC				64
#define CE_COMM_CTL_METHOD_SHIFT	0

#define CE_METHOD_IS_HASH(type) ((type == SS_METHOD_MD5) \
								|| (type == SS_METHOD_SHA1) \
								|| (type == SS_METHOD_SHA224) \
								|| (type == SS_METHOD_SHA256) \
								|| (type == SS_METHOD_SHA384) \
								|| (type == SS_METHOD_SHA512))

#define CE_METHOD_IS_AES(type) ((type == SS_METHOD_AES) \
								|| (type == SS_METHOD_DES) \
								|| (type == SS_METHOD_3DES))

/* About the symmetric control word */

#define CE_KEY_SELECT_INPUT			0
#define CE_KEY_SELECT_SSK			1
#define CE_KEY_SELECT_HUK			2
#define CE_KEY_SELECT_RSSK			3
#define CE_SYM_CTL_KEY_SELECT_SHIFT	20

#define CE_CFB_WIDTH_1				0
#define CE_CFB_WIDTH_8				1
#define CE_CFB_WIDTH_64				2
#define CE_CFB_WIDTH_128			3
#define CE_SYM_CTL_CFB_WIDTH_SHIFT	18

#define CE_SYM_CTL_AES_CTS_LAST		BIT(16)

#define SS_AES_MODE_ECB				0
#define SS_AES_MODE_CBC				1
#define SS_AES_MODE_CTR				2
#define SS_AES_MODE_CTS				3
#define SS_AES_MODE_OFB				4
#define SS_AES_MODE_CFB				5
#define SS_AES_MODE_CBC_MAC			6
#define CE_SYM_CTL_OP_MODE_SHIFT	8

#define CE_CTR_SIZE_16				0
#define CE_CTR_SIZE_32				1
#define CE_CTR_SIZE_64				2
#define CE_CTR_SIZE_128				3
#define CE_SYM_CTL_CTR_SIZE_SHIFT	2

#define CE_AES_KEY_SIZE_128			0
#define CE_AES_KEY_SIZE_192			1
#define CE_AES_KEY_SIZE_256			2
#define CE_SYM_CTL_KEY_SIZE_SHIFT	0

/* About the asymmetric control word */

#define CE_RSA_PUB_MODULUS_WIDTH_512	0
#define CE_RSA_PUB_MODULUS_WIDTH_1024	1
#define CE_RSA_PUB_MODULUS_WIDTH_2048	2
#define CE_RSA_PUB_MODULUS_WIDTH_3072	3
#define CE_RSA_PUB_MODULUS_WIDTH_4096	4
#define CE_ASYM_CTL_RSA_PM_WIDTH_SHIFT	28

#define CE_RSA_OP_M_EXP					0 /* modular exponentiation */
#define CE_RSA_OP_M_MUL					2 /* modular multiplication */
#define CE_ASYM_CTL_RSA_OP_SHIFT		16

#define CE_ECC_PARA_WIDTH_160			0
#define CE_ECC_PARA_WIDTH_224			2
#define CE_ECC_PARA_WIDTH_256			3
#define CE_ECC_PARA_WIDTH_521			5
#define CE_ASYM_CTL_ECC_PARA_WIDTH_SHIFT	12

#define CE_ECC_OP_POINT_MUL				0
#define CE_ECC_OP_POINT_ADD				1
#define CE_ECC_OP_POINT_DBL				2
#define CE_ECC_OP_POINT_VER				3
#define CE_ECC_OP_ENC					4
#define CE_ECC_OP_DEC					5
#define CE_ECC_OP_SIGN					6
#define CE_ASYM_CTL_ECC_OP_SHIFT		4

#define SS_SEED_SIZE			24

/* Function declaration */

u32 ss_reg_rd(u32 offset);
void ss_reg_wr(u32 offset, u32 val);

void ss_key_set(char *key, int size, ce_task_desc_t *task);

int ss_pending_get(void);
void ss_pending_clear(int flow);
void ss_irq_enable(int flow);
void ss_irq_disable(int flow);

void ss_iv_set(char *iv, int size, ce_task_desc_t *task);
void ss_iv_mode_set(int mode, ce_task_desc_t *task);

void ss_cnt_set(char *cnt, int size, ce_task_desc_t *task);
void ss_cnt_get(int flow, char *cnt, int size);

void ss_md_get(char *dst, char *src, int size);
void ss_sha_final(void);
void ss_check_sha_end(void);

void ss_rsa_width_set(int size, ce_task_desc_t *task);
void ss_rsa_op_mode_set(int mode, ce_task_desc_t *task);

void ss_ecc_width_set(int size, ce_task_desc_t *task);
void ss_ecc_op_mode_set(int mode, ce_task_desc_t *task);

void ss_cts_last(ce_task_desc_t *task);
void ss_hmac_sha1_last(ce_task_desc_t *task);

void ss_method_set(int dir, int type, ce_task_desc_t *task);

void ss_aes_mode_set(int mode, ce_task_desc_t *task);
void ss_cfb_bitwidth_set(int bitwidth, ce_task_desc_t *task);

void ss_wait_idle(void);
void ss_ctrl_start(ce_task_desc_t *task);
void ss_ctrl_stop(void);
int ss_flow_err(int flow);

void ss_data_len_set(int len, ce_task_desc_t *task);

int ss_reg_print(char *buf, int len);

#endif /* end of _SUNXI_SECURITY_SYSTEM_REG_H_ */

