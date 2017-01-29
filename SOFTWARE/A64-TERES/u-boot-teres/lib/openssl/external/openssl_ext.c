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
#include "common.h"
#include "linux/ctype.h"
#include "openssl_ext.h"
#include <asm/arch/ss.h>

extern void sid_read_rotpk(void *dst) ;
int sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len);
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int __asn1_probe_data_head(u8 *buf, sunxi_asn1_t *asn1)
{
	u8 *tmp_buf = buf;
	int index;
	int len, len_bytes;

	asn1->head     = tmp_buf[0];
	asn1->head_len = 2;
	//获取长度
	len = tmp_buf[1];
	if(len & 0x80)		//超过1个字节表示长度
	{
		len_bytes = len & 0x7f;
		if((!len_bytes) || (len_bytes>4))
		{
			printf("len_bytes(%d) is 0 or larger than 4, cant be probe\n", len_bytes);

			return -1;
		}
		asn1->head_len += len_bytes;
		index = 2;
		len = 0;
		while(--len_bytes);
		{
			len += tmp_buf[index++];
			len *= 256;
		}
		len |= tmp_buf[index];

	}
	asn1->data = buf + asn1->head_len;
	asn1->data_len = len;

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static  int __sunxi_publickey_dipatch(sunxi_key_t *pkey, u8 *buf, u32 len)
{
	u8 *tmp_buf = buf;
	int ret;
	sunxi_asn1_t asn1;

	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if(ret < 0)	//
	{
		printf("publickey_dipatch err: head is not a sequence\n");

		return -1;
	}
	tmp_buf += asn1.head_len;		//跳过sequnce头部
	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if((ret) || (asn1.head != 0x2))	//
	{
		printf("publickey_dipatch err: step 2\n");

		return -2;
	}
	pkey->n = malloc(asn1.data_len);
	memcpy(pkey->n, asn1.data, asn1.data_len);
	pkey->n_len = asn1.data_len;

	tmp_buf = asn1.data + asn1.data_len;
	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if((ret) || (asn1.head != 0x2))
	{
		printf("publickey_dipatch err: step 3\n");

		return -3;
	}

	pkey->e = malloc(asn1.data_len);
	memcpy(pkey->e, asn1.data, asn1.data_len);
	pkey->e_len = asn1.data_len;

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int __certif_probe_signdata(u8 *dst_buf, u32 dst_len_max, u8 *src_buf, u32 src_len)
{
	u8 *tmp_buf = src_buf;
	int ret;
	sunxi_asn1_t asn1;

	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if(ret < 0)	//
	{
		printf("certif_decode err: head is not a sequence\n");

		return -1;
	}
	tmp_buf += asn1.head_len;		//跳过sequnce头部
	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if(ret)
	{
		printf("certif_decode err: step 1\n");

		return -2;
	}

	if(asn1.data_len > dst_len_max)
	{
		printf("sign data len (0x%x) is longer then buffer size (0x%x)\n", asn1.data_len, dst_len_max);

		return -1;
	}
	memcpy(dst_buf, tmp_buf, asn1.data_len + asn1.head_len);

	return asn1.data_len + asn1.head_len;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static uint __merge_extension_value(u8 **dst_buf, u8 *src_buf, uint src_buf_len)
{
	u8 *tmp_buf = src_buf;
	sunxi_asn1_t asn1;
	int ret;
	uint tmp_len;

	ret = __asn1_probe_data_head(tmp_buf, &asn1);
	if(ret < 0)	//
	{
		printf("__merge_extension_value err: head is not a sequence\n");

		return 0;
	}

	if(asn1.data_len + asn1.head_len > src_buf_len)
	{
		printf("__merge_extension_value err: the data source len is too short\n");

		return 0;
	}
	*dst_buf = malloc((asn1.data_len + 1)/2);
	memset(*dst_buf, 0, (asn1.data_len + 1)/2);
	tmp_len = asn1.data_len;
	if(tmp_len > 512)		//rsakey
	{
		u8 *src = asn1.data;
		if((src[0] == '0') && (src[1] == '0'))
		{
			src += 2;
		}
		if(sunxi_bytes_merge(*dst_buf, asn1.data_len, src, 512))
		{
			printf("__merge_extension_value err1: in sunxi_bytes_merge\n");

			return 0;
		}
		if(sunxi_bytes_merge(*dst_buf + 512/2, asn1.data_len, src + 512, asn1.data_len - 512 - (src-asn1.data)))
		{
			printf("__merge_extension_value err2: in sunxi_bytes_merge\n");

			return 0;
		}
	}
	else
	{
		if(sunxi_bytes_merge(*dst_buf, asn1.data_len, asn1.data, asn1.data_len))
		{
			printf("__merge_extension_value err1: in sunxi_bytes_merge\n");

			return 0;
		}
	}

	//memcpy(*dst_buf, asn1.data, asn1.data_len);

	return (asn1.data_len + 1)/2;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_create(X509 **certif, u8 *buf, int len)
{
	u8 *p = buf;

	*certif = d2i_X509(NULL, (const unsigned char **)&p, len);
	if(*certif == NULL)
	{
		printf("x509_create: cant get a certif\n");

		return -1;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_free(X509 *certif)
{
	if(certif)
	{
		X509_free(certif);
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_probe_serial_num(X509 *x)
{
	ASN1_INTEGER *bs = NULL;
	long serial_num = 0;

	bs = X509_get_serialNumber(x);
	if(bs->length <= 4)
	{
		serial_num = ASN1_INTEGER_get(bs);
		printf("SERIANL NUMBER: 0x%x\n", (unsigned int)serial_num);
	}
	else
	{
		printf("SERIANL NUMBER: Unknown\n");
	}

	return 0 ;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_probe_version(X509 *x)
{
	long version = 0;

	version = X509_get_version(x);
	printf("Version: 0x%0x\n", (unsigned int)version);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
#define BUFF_NAME_MAX  128
#define BUFF_VALUE_MAX  3072

int sunxi_certif_probe_extension(X509 *x, sunxi_certif_info_t *sunxi_certif)
{
	int extension_count = X509_get_ext_count(x);
	X509_EXTENSION *extension;
	int i, len;
	ASN1_OBJECT *obj;
	u8 buff_name[BUFF_NAME_MAX];
	u8 buff_value[BUFF_VALUE_MAX];

	//printf("extension_count=%d\n", extension_count);
	sunxi_certif->extension.extension_num = extension_count;

	for(i = 0; i < extension_count; i++)
	{
		//printf("************%d***************\n", i);
		//printf("extension name:\n");
		extension=sk_X509_EXTENSION_value(x->cert_info->extensions, i);
		if(!extension)
		{
			printf("get extersion %d fail\n", i);

			return -1;
		}
		obj = X509_EXTENSION_get_object(extension);
		if(!obj)
		{
			printf("get extersion obj %d fail\n", i);

			return -1;
		}
		memset(buff_name, 0, BUFF_NAME_MAX);
		//while((*(volatile int *)0)!=12);
		//len = OBJ_obj2txt(buff_name, BUFF_NAME_MAX, obj, 0);
		len = OBJ_obj2name((char *)buff_name, BUFF_NAME_MAX, obj);
		if(!len)
		{
			printf("extersion %d name length is 0\n", i);
		}
		else
		{
			//printf("name len=%d\n", len);
			sunxi_certif->extension.name[i] = malloc(len + 1);
			memcpy(sunxi_certif->extension.name[i], buff_name, len);
			sunxi_certif->extension.name[i][len] = '\0';
			sunxi_certif->extension.name_len[i] = len;

			//xdump(sunxi_certif->extension.name[i], len);
		}

		memset(buff_value,0,BUFF_NAME_MAX);
		len = ASN1_STRING_mem((char *)buff_value, extension->value);
		if(!len)
		{
			printf("extersion %d value length is 0\n", i);
		}
		else
		{
			//xdump(buff_value, len);
			len = __merge_extension_value(&sunxi_certif->extension.value[i], buff_value, len);
			if(!len)
			{
				printf("get extension value failed\n");

				return -1;
			}
			sunxi_certif->extension.value_len[i] = len;
			//printf("value len=%d\n", len);

			//ndump(sunxi_certif->extension.value[i], len);
		}
		//printf("<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>\n");
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_probe_pubkey(X509 *x, sunxi_key_t *pubkey)
{
	EVP_PKEY *pkey = NULL;
	int keylen;
	char *buff_tmp;
//	int  sig_nid;
	u8  keybuff[512];

	pkey = X509_get_pubkey(x);
	if (pkey == NULL)
	{
		printf("cant find the public key %s %d\n", __FILE__, __LINE__);

		return -1;
	}
//	if(pkey->type == 6)
//	{
//		printf("it is rsaEncryption\n");
//	}
//	else
//	{
//		printf("unknown encryption\n");
//
//		//return -1;
//	}
//	sig_nid = OBJ_obj2nid(x->sig_alg->algorithm);
	memset(keybuff, 0, 512);
	buff_tmp = (char *)keybuff;
	keylen = i2d_PublicKey(pkey, (unsigned char **)&buff_tmp);
	if(keylen <= 0)
	{
		printf("The public key is invalid\n");

		return -1;
	}
	if(__sunxi_publickey_dipatch(pubkey, keybuff, keylen))
	{
		printf("get public failed\n");

		return -1;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_certif_mem_reset(void)
{
	reset_OBJ_nid2ln_reset();
	reset_CRYPTO_reset();
	reset_BIO_reset();
	reset_D2I_reset();
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_certif_probe_signature(X509 *x, u8 *sign)
{
	memcpy(sign, x->signature->data, x->signature->length);

	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  buf: 证书存放起始   len：数据长度
*
*    return        :
*
*    note          :  证书自校验
*
*
************************************************************************************************************
*/
int sunxi_certif_verify_itself(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len)
{
	X509 *certif;
	int  ret;
	u8   hash_of_certif[256];
	u8   hash_of_sign[256];

	u8   sign_in_certif[256];
	u8   *p_sign_to_calc;
	u32  sign_src_len;

	//内存初始化
	sunxi_certif_mem_reset();
	//创建证书
	ret = sunxi_certif_create(&certif, buf, len);
	if(ret < 0)
	{
		printf("fail to create a certif\n");

		return -1;
	}
	//获取证书公钥
	ret = sunxi_certif_probe_pubkey(certif, &sunxi_certif->pubkey);
	if(ret)
	{
		printf("fail to probe the public key\n");

		return -1;
	}
	//获取证书签名
	ret = sunxi_certif_probe_signature(certif, sign_in_certif);
	if(ret)
	{
		printf("fail to probe the sign value\n");

		return -1;
	}
	//获取需要签名内容
	//计算sha256时，必须保证内存起始位置16字节对齐，这里采取了32字节对齐
	p_sign_to_calc = malloc(4096);		//证书中待签名内容肯定不超过4k
	//获取待签名内容
	memset(p_sign_to_calc, 0, 4096);
	sign_src_len = __certif_probe_signdata(p_sign_to_calc, 4096, buf, len);
	if(sign_src_len <= 0)
	{
		printf("certif_probe_signdata err\n");

		return -1;
	}
	//计算待签名内容的hash
	memset(hash_of_certif, 0, sizeof(hash_of_certif));
	ret = sunxi_sha_calc(hash_of_certif, sizeof(hash_of_certif), p_sign_to_calc, sign_src_len);
	if(ret)
	{
		printf("sunxi_sha_calc: calc sha256 with hardware err\n");

		return -1;
	}
	//计算证书中签名的rsa
	memset(hash_of_sign, 0, sizeof(hash_of_sign));
	ret = sunxi_rsa_calc(sunxi_certif->pubkey.n+1, sunxi_certif->pubkey.n_len-1,
	                     sunxi_certif->pubkey.e, sunxi_certif->pubkey.e_len,
	                     hash_of_sign,           sizeof(hash_of_sign),
	                     sign_in_certif,         sizeof(sign_in_certif));
	if(ret)
	{
		printf("sunxi_rsa_calc: calc rsa2048 with hardware err\n");

		return -1;
	}
//	printf(">>>>>>>>>>>>>>hash_of_certif\n");
//	ndump(hash_of_certif, 32);
//	printf("<<<<<<<<<<<<<<\n");
//	printf(">>>>>>>>>>>>>>hash_of_sign\n");
//	ndump(hash_of_sign, 32);
//	printf("<<<<<<<<<<<<<<\n");
	if(memcmp(hash_of_certif, hash_of_sign, 32))
	{
		printf("certif verify failed\n");

		return -1;
	}
	ret = sunxi_certif_probe_extension(certif, sunxi_certif);
	if(ret)
	{
		printf("sunxi_rsa_calc: probe extension failed\n");

		return -1;
	}

	sunxi_certif_free(certif);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  buf: 证书存放起始   len：数据长度
*
*    return        :
*
*    note          :  证书自校验
*
*
************************************************************************************************************
*/
int sunxi_certif_probe_ext(sunxi_certif_info_t *sunxi_certif, u8 *buf, u32 len)
{
	X509 *certif;
	int  ret;
	//内存初始化
	sunxi_certif_mem_reset();
	//创建证书
	ret = sunxi_certif_create(&certif, buf, len);
	if(ret < 0)
	{
		printf("fail to create a certif\n");

		return -1;
	}
	ret = sunxi_certif_probe_extension(certif, sunxi_certif);
	if(ret)
	{
		printf("sunxi_rsa_calc: probe extension failed\n");

		return -1;
	}
	sunxi_certif_free(certif);

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :  对于一个序列，按照高4+低4,合并成为一个新的字节，比如
*                     0x41(A) 0x31(1)  合并成为0xa1
*
************************************************************************************************************
*/
static int __sample_atoi(u8 ch, u8 *dst)
{
	u8 ret_c;

	if(isdigit(ch))
		ret_c = ch - '0';
	else if(isupper(ch))
		ret_c = ch - 'A' + 10;
	else if(islower(ch))
		ret_c = ch - 'a' + 10;
	else
	{
		printf("sample_atoi err: ch 0x%02x is not a digit or hex ch\n", ch);
		return -1;
	}
	*dst = ret_c;

	return 0;
}

int sunxi_bytes_merge(u8 *dst, u32 dst_len, u8 *src, uint src_len)
{
	int i=0, j;
	u8  c_h, c_l;

	if((src_len>>1) > dst_len)
	{
		printf("bytes merge failed, the dst buffer is too short\n");

		return -1;
	}
	if(src_len & 0x01)		//奇数
	{
		src_len --;
		if(__sample_atoi(src[i], &dst[0]))
		{
			return -1;
		}
		i++;
	}

	for(j=i;i<src_len;i+=2, j++)
	{
		c_h = src[i];
		c_l = src[i+1];

		if(__sample_atoi(src[i], &c_h))
		{
			return -1;
		}

		if(__sample_atoi(src[i+1], &c_l))
		{
			return -1;
		}
		dst[j] = (c_h << 4) | c_l;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :  buf: 证书存放起始   len：数据长度
*
*    return        :
*
*    note          :  证书自校验
*
*
************************************************************************************************************
*/
int sunxi_certif_dump(sunxi_certif_info_t *sunxi_certif)
{
	return 0;
}

