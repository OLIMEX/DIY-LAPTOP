
#include "obj_mac.h"

#define NUM_NID 23				
#define NUM_SN 23				
#define NUM_LN 23					
#define NUM_OBJ 23				

static  unsigned char lvalues[200]={
	0x00,                                        					        /* [  0] OBJ_undef ,1*/
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,						/* [ 1] OBJ_rsaEncryption,9 *///####
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,          							/* [10] OBJ_netscape ,7*/
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,     							/* [17] OBJ_netscape_cert_extension,8 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x01,							/* [25] OBJ_netscape_cert_type,9 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x02,							/* [34] OBJ_netscape_base_url,9 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x03,							/* [43] OBJ_netscape_revocation_url,9 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x04,							/* [52] OBJ_netscape_ca_revocation_url ,9*/
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x07,							/* [61] OBJ_netscape_renewal_url ,9*/
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x08,							/* [70] OBJ_netscape_ca_policy_url,9 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x0C,							/* [79] OBJ_netscape_ssl_server_name,9 */
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x0D,							/* [88] OBJ_netscape_comment ,9*/
	0x60,0x86,0x48,0x01,0x86,0xF8,0x42,0x01,0x0E,							/*[97]OBJ_AW_comment##,9*/	//samayng  modify
	0x60,0x86,0x48,0x01,0x86,0xf7,0x0d,										/*[106]OBJ_AW_extension,7*/
	0x60,0x86,0x48,0x01,0x86,0xf7,0x0d,0x83,0xff,0X7f,0X83,0xff,0x7f,		/*[113]OBJ_AW_comment,13*/
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,        					/* [126] OBJ_pkcs,7 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,     					/* [133] OBJ_pkcs1 ,8*/
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x04,					/* [141] OBJ_md5WithRSAEncryption,9 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,					/* [150] OBJ_sha256WithRSAEncryption,9 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0C,					/* [159] OBJ_sha384WithRSAEncryption,9 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0D,					/* [168] OBJ_sha512WithRSAEncryption,9 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0E,					/* [177] OBJ_sha224WithRSAEncryption,9 */
	0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,					/* [186] OBJ_sha1WithRSAEncryption,9 */


};
//static const ASN1_OBJECT nid_objs[NUM_NID]={//--hgl--20140403--two_certif test
static  ASN1_OBJECT nid_objs[NUM_NID]={
	{"UNDEF","undefined",NID_undef,1,&(lvalues[0]),0},
	{"rsaEncryption","rsaEncryption",NID_rsaEncryption,9,&(lvalues[1]),0},
	{"Netscape","Netscape Communications Corp.",NID_netscape,7, &(lvalues[10]),0},
	{"nsCertExt","Netscape Certificate Extension",	NID_netscape_cert_extension,8,&(lvalues[17]),0},
	{"nsCertType","Netscape Cert Type",NID_netscape_cert_type,9,&(lvalues[25]),0},
	{"samyang7","test7",NID_netscape_base_url,9,&(lvalues[34]),0},
	{"samyang6","test6",NID_netscape_revocation_url,9,&(lvalues[43]),0},
	{"samyang5","test5",NID_netscape_ca_revocation_url,9,&(lvalues[52]),0},
	{"samyang4","test4",NID_netscape_renewal_url,9,&(lvalues[61]),0},
	{"samyang3","test3",NID_netscape_ca_policy_url,9,&(lvalues[70]),0},
	{"samyang2","test2",NID_netscape_ssl_server_name,9,&(lvalues[79]),0},
	{"samyang1","test1",NID_netscape_comment,9,&(lvalues[88]),0},
	{"samyang8","Samyang_comment",NID_test_comment,9,&(lvalues[97]),0},
	{"AW","ALLWINNER  EXTENSION",NID_aw_cert_extension,7,&(lvalues[106]),0},
	{"awcomment1","allwinner comment1", NID_aw_comment1	,13,&(lvalues[113]),0},
	{"pkcs","RSA Data Security, Inc. PKCS",NID_pkcs,7,&(lvalues[126]),0},
	{"pkcs1","pkcs1",NID_pkcs1,8,&(lvalues[133]),0},
	{"RSA-MD5","md5WithRSAEncryption",NID_md5WithRSAEncryption,9,&(lvalues[141]),0},
	{"RSA-SHA256","sha256WithRSAEncryption",NID_sha256WithRSAEncryption,9,&(lvalues[150]),0},
	{"RSA-SHA384","sha384WithRSAEncryption",NID_sha384WithRSAEncryption,9,&(lvalues[159]),0},
	{"RSA-SHA512","sha512WithRSAEncryption",NID_sha512WithRSAEncryption,9,&(lvalues[168]),0},
	{"RSA-SHA224","sha224WithRSAEncryption",NID_sha224WithRSAEncryption,9,&(lvalues[177]),0},
	{"RSA-SHA1","sha1WithRSAEncryption",NID_sha1WithRSAEncryption,9,&(lvalues[186]),0},



};



static ASN1_OBJECT *sn_objs[NUM_LN]={
	&(nid_objs[ 0]),/* "undefined" */
	&(nid_objs[ 1]),/* "rsaEncryption" */
	&(nid_objs[2]),/* "Netscape" */
	&(nid_objs[3]),/* "nsCertExt" */
	&(nid_objs[4]),/* "nsCertType" */
	&(nid_objs[5]),/* "samyang7" */
	&(nid_objs[6]),/* "samyang6" */
	&(nid_objs[7]),/* "samyang5" */
	&(nid_objs[8]),/* "samyang4" */
	&(nid_objs[9]),/* "samyang3" */
	&(nid_objs[10]),/* "samyang2" */
	&(nid_objs[11]),/* "samyang1" */
	&(nid_objs[12]),/* "samyang8" */
	&(nid_objs[13]),/* "AW" */
	&(nid_objs[14]),/* "awcomment1" */
	&(nid_objs[15]),/* OBJ_pkcs                         1 2 840 113549 1 */
	&(nid_objs[16]),/* OBJ_pkcs1                        1 2 840 113549 1 1 */
	&(nid_objs[17]),/* OBJ_md5WithRSAEncryption         1 2 840 113549 1 1 4 */
	&(nid_objs[18]),/* OBJ_sha256WithRSAEncryption      1 2 840 113549 1 1 11 */
	&(nid_objs[19]),/* OBJ_sha384WithRSAEncryption      1 2 840 113549 1 1 12 */
	&(nid_objs[20]),/* OBJ_sha512WithRSAEncryption      1 2 840 113549 1 1 13 */
	&(nid_objs[21]),/* OBJ_sha224WithRSAEncryption      1 2 840 113549 1 1 14 */
	&(nid_objs[22]),/* OBJ_sha1WithRSAEncryption        1 2 840 113549 1 1 5 */
};

static ASN1_OBJECT *ln_objs[NUM_SN]={
	&(nid_objs[ 0]),/* "UNDEF" */
	&(nid_objs[ 1]),/* "rsaEncryption" */
	&(nid_objs[2]),/* "Netscape" */
	&(nid_objs[3]),/* "nsCertExt" */
	&(nid_objs[4]),/* "nsCertType" */
	&(nid_objs[5]),/* "samyang7" */
	&(nid_objs[6]),/* "samyang6" */
	&(nid_objs[7]),/* "samyang5" */
	&(nid_objs[8]),/* "samyang4" */
	&(nid_objs[9]),/* "samyang3" */
	&(nid_objs[10]),/* "samyang2" */
	&(nid_objs[11]),/* "samyang1" */

	&(nid_objs[12]),/* "samyang8" */
	&(nid_objs[13]),/* "AW" */
	&(nid_objs[14]),/* "awcomment1" */

	&(nid_objs[15]),/* OBJ_pkcs                         1 2 840 113549 1 */
	&(nid_objs[16]),/* OBJ_pkcs1                        1 2 840 113549 1 1 */
	&(nid_objs[17]),/* OBJ_md5WithRSAEncryption         1 2 840 113549 1 1 4 */
	&(nid_objs[18]),/* OBJ_sha256WithRSAEncryption      1 2 840 113549 1 1 11 */
	&(nid_objs[19]),/* OBJ_sha384WithRSAEncryption      1 2 840 113549 1 1 12 */
	&(nid_objs[20]),/* OBJ_sha512WithRSAEncryption      1 2 840 113549 1 1 13 */
	&(nid_objs[21]),/* OBJ_sha224WithRSAEncryption      1 2 840 113549 1 1 14 */
	&(nid_objs[22]),/* OBJ_sha1WithRSAEncryption        1 2 840 113549 1 1 5 */


};

static ASN1_OBJECT *obj_objs[NUM_OBJ]={
	&(nid_objs[ 0]),/* OBJ_undef                        0 */
	&(nid_objs[ 1]),/* "rsaEncryption" */
	&(nid_objs[2]),/* "Netscape" */
	&(nid_objs[3]),/* "nsCertExt" */
	&(nid_objs[4]),/* "nsCertType" */
	&(nid_objs[5]),/* "samyang7" */
	&(nid_objs[6]),/* "samyang6" */
	&(nid_objs[7]),/* "samyang5" */
	&(nid_objs[8]),/* "samyang4" */
	&(nid_objs[9]),/* "samyang3" */
	&(nid_objs[10]),/* "samyang2" */
	&(nid_objs[11]),/* "samyang1" */

	&(nid_objs[12]),/* "samyang8" */
	&(nid_objs[13]),/* "AW" */
	&(nid_objs[14]),/* "awcomment1" */

	&(nid_objs[15]),/* OBJ_pkcs                         1 2 840 113549 1 */
	&(nid_objs[16]),/* OBJ_pkcs1                        1 2 840 113549 1 1 */
	&(nid_objs[17]),/* OBJ_md5WithRSAEncryption         1 2 840 113549 1 1 4 */
	&(nid_objs[18]),/* OBJ_sha256WithRSAEncryption      1 2 840 113549 1 1 11 */
	&(nid_objs[19]),/* OBJ_sha384WithRSAEncryption      1 2 840 113549 1 1 12 */
	&(nid_objs[20]),/* OBJ_sha512WithRSAEncryption      1 2 840 113549 1 1 13 */
	&(nid_objs[21]),/* OBJ_sha224WithRSAEncryption      1 2 840 113549 1 1 14 */
	&(nid_objs[22]),/* OBJ_sha1WithRSAEncryption        1 2 840 113549 1 1 5 */



};

