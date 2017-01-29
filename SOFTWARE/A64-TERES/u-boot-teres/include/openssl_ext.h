/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2014, Allwinner Technology Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#ifndef  _SUNXI_OPENSSL_EXT_H__
#define  _SUNXI_OPENSSL_EXT_H__

#include "x509.h"
#include "bn.h"

#define  SUNXI_EXTENSION_ITEM_MAX   (8)

typedef struct
{
	int  extension_num;
	u8   *name[SUNXI_EXTENSION_ITEM_MAX];
	uint name_len[SUNXI_EXTENSION_ITEM_MAX];
	u8   *value[SUNXI_EXTENSION_ITEM_MAX];
	uint value_len[SUNXI_EXTENSION_ITEM_MAX];
}
sunxi_extension_t;


typedef struct
{
	u8 *n;
	u32 n_len;
	u8 *e;
	u32 e_len;
}
sunxi_key_t;


typedef struct
{
	long version;
	long serial_num;
	sunxi_key_t        pubkey;
	sunxi_extension_t  extension;
}
sunxi_certif_info_t;


typedef struct
{
	int head;
	int head_len;
	u8 *data;
	int data_len;
}
sunxi_asn1_t;

int sunxi_certif_create(X509 **certif, u8 *buf, int len);
int sunxi_certif_free(X509 *certif);
int sunxi_certif_probe_serial_num(X509 *x);
int sunxi_certif_probe_version(X509 *x);
int sunxi_certif_probe_extension(X509 *x, sunxi_certif_info_t *sunxi_certif);

int sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len);

int sunxi_certif_probe_ext(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len);
int sunxi_certif_verify_itself(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len);

int OBJ_obj2name(char *dst_buf, int buf_len, const ASN1_OBJECT *a);
int ASN1_STRING_mem(char *bp, const ASN1_STRING *v);

void reset_BIO_reset(void);
void reset_OBJ_nid2ln_reset(void);
void reset_CRYPTO_reset(void);
void reset_D2I_reset(void);

#endif
