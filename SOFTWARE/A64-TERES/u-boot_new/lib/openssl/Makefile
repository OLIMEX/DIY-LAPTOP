
##
## Makefile for Sunxi Secure Boot
##

include $(SPLDIR)/config.mk


LIB	:= $(obj)libopenssl.o

COBJS-y +=  CRYPTO_malloc.o

COBJS-y +=  err/err.o

COBJS-y +=  objects/obj_dat.o

COBJS-y +=  asn1/a_int.o asn1/asn1_get_field_ptr.o asn1/ASN1_INTEGER_get.o asn1/asn1_item_ex_new.o  \
            asn1/asn1_lock_free.o asn1/asn1_new_free.o asn1/asn1_put_object.o asn1/asn1_string.o    \
            asn1/ASN1_STRING_print.o asn1/d2i_pu.o asn1/d2i_rsapublikey.o asn1/EVP_pkey.o           \
            asn1/i2a_ASN1_INTEGER.o asn1/i2d_pu.o asn1/X509_PUBKEY_it.o asn1/x_bignum.o             \
            asn1/x_x509.o asn1/RSAPublicKey_it.o  asn1/tasn_enc.o asn1/stk.o asn1/x509_extension_it.o

COBJS-y +=  x509/x509_v3.o x509/x509_ext.o x509/extension.o

COBJS-y +=  bn/bn_lib.o bn/bn_word.o bn/bn_bn2dec.o bn/bn_print.o bn/bn_asm.o bn/bn_shift.o

COBJS-y +=  bio/bio.o bio/b_print.o bio/bio_free.o bio/bio_s_mem.o bio/bio_read.o

COBJS-y +=  lhash/lh_retrieve.o

COBJS-y +=  rsa/rsa.o

COBJS-y +=  external/openssl_ext.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
