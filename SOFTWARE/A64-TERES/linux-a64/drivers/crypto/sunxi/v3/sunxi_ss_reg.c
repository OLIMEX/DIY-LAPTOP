/*
 * The interface function of controlling the SS register.
 *
 * Copyright (C) 2013 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "../sunxi_ss.h"
#include "sunxi_ss_reg.h"

inline u32 ss_readl(u32 offset)
{
	return readl(ss_membase() + offset);
}

inline void ss_writel(u32 offset, u32 val)
{
	writel(val, ss_membase() + offset);
}

u32 ss_reg_rd(u32 offset)
{
	return ss_readl(offset);
}

void ss_reg_wr(u32 offset, u32 val)
{
	ss_writel(offset, val);
}

void ss_keyselect_set(int select, ce_task_desc_t *task)
{
	task->sym_ctl |= select << CE_SYM_CTL_KEY_SELECT_SHIFT;
}

void ss_keysize_set(int size, ce_task_desc_t *task)
{
	int type = CE_AES_KEY_SIZE_128;

	switch (size) {
	case AES_KEYSIZE_192:
		type = CE_AES_KEY_SIZE_192;
		break;
	case AES_KEYSIZE_256:
		type = CE_AES_KEY_SIZE_256;
		break;
	default:
//		type = CE_AES_KEY_SIZE_128;
		break;
	}
	
	task->sym_ctl |= (type << CE_SYM_CTL_KEY_SIZE_SHIFT);
}

/* key: phsical address. */
void ss_key_set(char *key, int size, ce_task_desc_t *task)
{
	ss_keyselect_set(CE_KEY_SELECT_INPUT, task);
	ss_keysize_set(size, task);
	task->key_addr = virt_to_phys(key);
}

void ss_pending_clear(int flow)
{
	int val = CE_CHAN_PENDING << flow;
	ss_writel(CE_REG_ISR, val);
}

int ss_pending_get(void)
{
	return ss_readl(CE_REG_ISR);
}

void ss_irq_enable(int flow)
{
	int val = ss_readl(CE_REG_ICR);

	val |= CE_CHAN_INT_ENABLE << flow;
	ss_writel(CE_REG_ICR, val);
}

void ss_irq_disable(int flow)
{
	int val = ss_readl(CE_REG_ICR);

	val &= ~(CE_CHAN_INT_ENABLE << flow);
	ss_writel(CE_REG_ICR, val);
}

void ss_md_get(char *dst, char *src, int size)
{
	memcpy(dst, src, size);
}

/* iv: phsical address. */
void ss_iv_set(char *iv, int size, ce_task_desc_t *task)
{
	task->iv_addr = virt_to_phys(iv);
}

void ss_iv_mode_set(int mode, ce_task_desc_t *task)
{
	task->comm_ctl |= mode << CE_COMM_CTL_IV_MODE_SHIFT;
}

void ss_cntsize_set(int size, ce_task_desc_t *task)
{
	task->sym_ctl |= size << CE_SYM_CTL_CTR_SIZE_SHIFT;
}

void ss_cnt_set(char *cnt, int size, ce_task_desc_t *task)
{
	task->ctr_addr = virt_to_phys(cnt);
	
	ss_cntsize_set(CE_CTR_SIZE_128, task);
}

void ss_cts_last(ce_task_desc_t *task)
{
	task->sym_ctl |= CE_SYM_CTL_AES_CTS_LAST;
}

void ss_hmac_sha1_last(ce_task_desc_t *task)
{
	task->comm_ctl |= CE_HMAC_SHA1_LAST;
}

void ss_method_set(int dir, int type, ce_task_desc_t *task)
{
	task->comm_ctl |= dir << CE_COMM_CTL_OP_DIR_SHIFT;
	task->comm_ctl |= type << CE_COMM_CTL_METHOD_SHIFT;
}

void ss_aes_mode_set(int mode, ce_task_desc_t *task)
{
	task->sym_ctl |= mode << CE_SYM_CTL_OP_MODE_SHIFT;
}

void ss_cfb_bitwidth_set(int bitwidth, ce_task_desc_t *task)
{
	int val = 0;

	switch (bitwidth) {
	case 1:
		val = CE_CFB_WIDTH_1;
		break;
	case 8:
		val = CE_CFB_WIDTH_8;
		break;
	case 64:
		val = CE_CFB_WIDTH_64;
		break;
	case 128:
		val = CE_CFB_WIDTH_128;
		break;
	default:
		break;
	}
	task->sym_ctl |= val << CE_SYM_CTL_CFB_WIDTH_SHIFT;
}

void ss_sha_final(void)
{
	/* unsupported. */
}

void ss_check_sha_end(void)
{
	/* unsupported. */
}

void ss_rsa_width_set(int size, ce_task_desc_t *task)
{
	int width_type = CE_RSA_PUB_MODULUS_WIDTH_512;

	switch (size*8) {
	case 512:
		width_type = CE_RSA_PUB_MODULUS_WIDTH_512;
		break;
	case 1024:
		width_type = CE_RSA_PUB_MODULUS_WIDTH_1024;
		break;
	case 2048:
		width_type = CE_RSA_PUB_MODULUS_WIDTH_2048;
		break;
	case 3072:
		width_type = CE_RSA_PUB_MODULUS_WIDTH_3072;
		break;
	case 4096:
		width_type = CE_RSA_PUB_MODULUS_WIDTH_4096;
		break;
	default:
		break;
	}

	task->asym_ctl |= width_type<<CE_ASYM_CTL_RSA_PM_WIDTH_SHIFT;
}

void ss_rsa_op_mode_set(int mode, ce_task_desc_t *task)
{
	/* Consider !2 as M_EXP, for compatible with the previous SOC. */
	if (mode == CE_RSA_OP_M_MUL)
		task->asym_ctl |= CE_RSA_OP_M_MUL<<CE_ASYM_CTL_RSA_OP_SHIFT;
	else
		task->asym_ctl |= CE_RSA_OP_M_EXP<<CE_ASYM_CTL_RSA_OP_SHIFT;
}

void ss_ecc_width_set(int size, ce_task_desc_t *task)
{
	int width_type = CE_ECC_PARA_WIDTH_160;

	switch (size*8) {
	case 224:
		width_type = CE_ECC_PARA_WIDTH_224;
		break;
	case 256:
		width_type = CE_ECC_PARA_WIDTH_256;
		break;
	case 544: /* align with word */
		width_type = CE_ECC_PARA_WIDTH_521;
		break;
	default:
		break;
	}

	task->asym_ctl |= width_type<<CE_ASYM_CTL_ECC_PARA_WIDTH_SHIFT;
}

void ss_ecc_op_mode_set(int mode, ce_task_desc_t *task)
{
	task->asym_ctl |= mode<<CE_ASYM_CTL_ECC_OP_SHIFT;
}

void ss_ctrl_start(ce_task_desc_t *task)
{
	ss_writel(CE_REG_TSK, virt_to_phys(task));
	ss_writel(CE_REG_TLR, 0x1);	
}

void ss_ctrl_stop(void)
{
	/* unsupported */
}

int ss_flow_err(int flow)
{
	return ss_readl(CE_REG_ERR) & CE_REG_ESR_CHAN_MASK(flow);
}

void ss_wait_idle(void)
{
	while ((ss_readl(CE_REG_TSR) & CE_REG_TSR_BUSY_MASK) == CE_REG_TSR_BUSY) {
//		SS_DBG("Need wait for the hardware.\n");
		msleep(10);
	}
}

void ss_data_len_set(int len, ce_task_desc_t *task)
{
	task->data_len = len;
}

int ss_reg_print(char *buf, int len)
{
	return snprintf(buf, len,
		"The SS control register: \n"
		"[TSK] 0x%02x = 0x%08x \n"
		"[CTL] 0x%02x = 0x%08x \n"
		"[ICR] 0x%02x = 0x%08x, [ISR] 0x%02x = 0x%08x \n"
		"[TLR] 0x%02x = 0x%08x \n"
		"[TSR] 0x%02x = 0x%08x \n"
		"[ERR] 0x%02x = 0x%08x \n"
		"[CSS] 0x%02x = 0x%08x, [CDS] 0x%02x = 0x%08x \n"
		"[CSA] 0x%02x = 0x%08x, [CDA] 0x%02x = 0x%08x \n"
		"[TPR] 0x%02x = 0x%08x \n",
		CE_REG_TSK, ss_readl(CE_REG_TSK),
		CE_REG_CTL, ss_readl(CE_REG_CTL),
		CE_REG_ICR, ss_readl(CE_REG_ICR),
		CE_REG_ISR, ss_readl(CE_REG_ISR),
		CE_REG_TLR, ss_readl(CE_REG_TLR),
		CE_REG_TSR, ss_readl(CE_REG_TSR),
		CE_REG_ERR, ss_readl(CE_REG_ERR),
		CE_REG_CSS, ss_readl(CE_REG_CSS),
		CE_REG_CDS, ss_readl(CE_REG_CDS),
		CE_REG_CSA, ss_readl(CE_REG_CSA),
		CE_REG_CDA, ss_readl(CE_REG_CDA),
		CE_REG_TPR, ss_readl(CE_REG_TPR));
}

