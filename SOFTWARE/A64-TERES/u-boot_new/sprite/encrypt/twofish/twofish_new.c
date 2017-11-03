/* This is an independent implementation of the encryption algorithm:   */
/*                                                                      */
/*         Twofish by Bruce Schneier and colleagues                     */
/*                                                                      */
/* which is a candidate algorithm in the Advanced Encryption Standard   */
/* programme of the US National Institute of Standards and Technology.  */
/*                                                                      */
/* Copyright in this implementation is held by Dr B R Gladman but I     */
/* hereby give permission for its free direct or derivative use subject */
/* to acknowledgment of its origin and compliance with any conditions   */
/* that the originators of t he algorithm place on its exploitation.     */
/*                                                                      */
/* My thanks to Doug Whiting and Niels Ferguson for comments that led   */
/* to improvements in this implementation.                              */
/*                                                                      */
/* Dr Brian Gladman (gladman@seven77.demon.co.uk) 14th January 1999     */
/* Slightly modificatied for libzdt by Jeremy Tregunna. */

/* $Id: twofish.c,v 1.1.1.1 1999/10/15 22:49:23 nmav Exp $ */

/* Timing data for Twofish (twofish.c)

   128 bit key:
   Key Setup:    8414 cycles
   Encrypt:       376 cycles =    68.1 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          375 cycles =    68.3 mbits/sec

   192 bit key:
   Key Setup:   11628 cycles
   Encrypt:       376 cycles =    68.1 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          375 cycles =    68.3 mbits/sec

   256 bit key:
   Key Setup:   15457 cycles
   Encrypt:       381 cycles =    67.2 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          378 cycles =    67.8 mbits/sec

 */

#include "twofish_new.h"


/* u32  k_len;
 * u32  l_key[40];
 * u32  s_key[4];
 */

/* Extract byte from a 32 bit quantity (little endian notation)     */
#define byte(x,n)   ((unsigned char)((x) >> (n << 3)))

/* finite field arithmetic for GF(2**8) with the modular    */
/* polynomial x^8 + x^6 + x^5 + x^3 + 1 (0x169)             */

#define G_M 0x0169

unsigned char tab_5b[4] = { 0, G_M >> 2, G_M >> 1, (G_M >> 1) ^ (G_M >> 2) };
unsigned char tab_ef[4] = { 0, (G_M >> 1) ^ (G_M >> 2), G_M >> 1, G_M >> 2 };

#define ffm_01(x)    (x)
#define ffm_5b(x)   ((x) ^ ((x) >> 2) ^ tab_5b[(x) & 3])
#define ffm_ef(x)   ((x) ^ ((x) >> 1) ^ ((x) >> 2) ^ tab_ef[(x) & 3])

unsigned char ror4[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
unsigned char ashx[16] = { 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12, 5, 14, 7 };

unsigned char qt0[2][16] = {
	{8, 1, 7, 13, 6, 15, 3, 2, 0, 11, 5, 9, 14, 12, 10, 4},
	{2, 8, 11, 13, 15, 7, 6, 14, 3, 1, 9, 4, 0, 10, 12, 5}
};

unsigned char qt1[2][16] = {
	{14, 12, 11, 8, 1, 2, 3, 5, 15, 4, 10, 6, 7, 0, 9, 13},
	{1, 14, 2, 11, 4, 12, 3, 7, 6, 13, 10, 5, 15, 9, 0, 8}
};

unsigned char qt2[2][16] = {
	{11, 10, 5, 14, 6, 13, 9, 0, 12, 8, 15, 3, 2, 4, 7, 1},
	{4, 12, 7, 5, 1, 6, 9, 10, 0, 14, 13, 8, 2, 11, 3, 15}
};

unsigned char qt3[2][16] = {
	{13, 7, 15, 4, 1, 2, 6, 14, 9, 11, 3, 0, 8, 5, 12, 10},
	{11, 9, 5, 1, 12, 3, 13, 14, 6, 4, 7, 15, 2, 0, 8, 10}
};

unsigned char
qp(const uint n,const unsigned char x)
{
	unsigned char a0,a1,a2,a3,a4,b0,b1,b2,b3,b4;

	a0 = x >> 4;
	b0 = x & 15;
	a1 = a0 ^ b0;
	b1 = ror4[b0] ^ ashx[a0];
	a2 = qt0[n][a1];
	b2 = qt1[n][b1];
	a3 = a2 ^ b2;
	b3 = ror4[b2] ^ ashx[a2];
	a4 = qt2[n][a3];
	b4 = qt3[n][b3];
	return (b4 << 4) | a4;
}


#define q(n,x)  pkey->q_tab[n][x]

void
gen_qtab(TWI *pkey)
{
	uint i;

	for(i = 0; i < 256; ++i) {
		q(0,i) = qp(0,(unsigned char)i);
		q(1,i) = qp(1,(unsigned char)i);
	}
}





void gen_mtab(TWI *pkey)
{
	uint i,f01,f5b,fef;

	for(i = 0; i < 256; ++i) {
		f01 = q(1,i);
		f5b = ffm_5b(f01);
		fef = ffm_ef(f01);
		pkey->m_tab[0][i] =
		    f01 + (f5b << 8) + (fef << 16) + (fef << 24);
		pkey->m_tab[2][i] =
		    f5b + (fef << 8) + (f01 << 16) + (fef << 24);

		f01 = q(0,i);
		f5b = ffm_5b(f01);
		fef = ffm_ef(f01);
		pkey->m_tab[1][i] =
		    fef + (fef << 8) + (f5b << 16) + (f01 << 24);
		pkey->m_tab[3][i] =
		    f5b + (f01 << 8) + (fef << 16) + (f5b << 24);
	}
}

#define mds(n,x)    pkey->m_tab[n][x]

uint h_fun(TWI *pkey,const uint x,const uint key[])
{
	uint b0,b1,b2,b3;



	b0 = byte(x, 0);
	b1 = byte(x, 1);
	b2 = byte(x, 2);
	b3 = byte(x, 3);

	switch (pkey->k_len) {
	case 4:
		b0 = q(1, b0) ^ byte(key[3], 0);
		b1 = q(0, b1) ^ byte(key[3], 1);
		b2 = q(0, b2) ^ byte(key[3], 2);
		b3 = q(1, b3) ^ byte(key[3], 3);
	case 3:
		b0 = q(1, b0) ^ byte(key[2], 0);
		b1 = q(1, b1) ^ byte(key[2], 1);
		b2 = q(0, b2) ^ byte(key[2], 2);
		b3 = q(0, b3) ^ byte(key[2], 3);
	case 2:
		b0 = q(0, q(0, b0) ^ byte(key[1], 0)) ^ byte(key[0], 0);
		b1 = q(0, q(1, b1) ^ byte(key[1], 1)) ^ byte(key[0], 1);
		b2 = q(1, q(0, b2) ^ byte(key[1], 2)) ^ byte(key[0], 2);
		b3 = q(1, q(1, b3) ^ byte(key[1], 3)) ^ byte(key[0], 3);
	}


	return mds(0, b0) ^ mds(1, b1) ^ mds(2, b2) ^ mds(3, b3);
}



#define q20(x)  q(0,q(0,x) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q21(x)  q(0,q(1,x) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q22(x)  q(1,q(0,x) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q23(x)  q(1,q(1,x) ^ byte(key[1],3)) ^ byte(key[0],3)

#define q30(x)  q(0,q(0,q(1, x) ^ byte(key[2],0)) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q31(x)  q(0,q(1,q(1, x) ^ byte(key[2],1)) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q32(x)  q(1,q(0,q(0, x) ^ byte(key[2],2)) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q33(x)  q(1,q(1,q(0, x) ^ byte(key[2],3)) ^ byte(key[1],3)) ^ byte(key[0],3)

#define q40(x)  q(0,q(0,q(1, q(1, x) ^ byte(key[3],0)) ^ byte(key[2],0)) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q41(x)  q(0,q(1,q(1, q(0, x) ^ byte(key[3],1)) ^ byte(key[2],1)) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q42(x)  q(1,q(0,q(0, q(0, x) ^ byte(key[3],2)) ^ byte(key[2],2)) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q43(x)  q(1,q(1,q(0, q(1, x) ^ byte(key[3],3)) ^ byte(key[2],3)) ^ byte(key[1],3)) ^ byte(key[0],3)

void  gen_mk_tab(TWI *pkey, uint key[])
{
	uint i;
	unsigned char by;

	switch (pkey->k_len) {
	case 2:
		for (i = 0; i < 256; ++i) {
			by = (unsigned char) i;

			pkey->mk_tab[0][i] = mds(0, q20(by));
			pkey->mk_tab[1][i] = mds(1, q21(by));
			pkey->mk_tab[2][i] = mds(2, q22(by));
			pkey->mk_tab[3][i] = mds(3, q23(by));

		}
		break;

	case 3:
		for (i = 0; i < 256; ++i) {
			by = (unsigned char) i;

			pkey->mk_tab[0][i] = mds(0, q30(by));
			pkey->mk_tab[1][i] = mds(1, q31(by));
			pkey->mk_tab[2][i] = mds(2, q32(by));
			pkey->mk_tab[3][i] = mds(3, q33(by));

		}
		break;

	case 4:
		for (i = 0; i < 256; ++i) {
			by = (unsigned char) i;

			pkey->mk_tab[0][i] = mds(0, q40(by));
			pkey->mk_tab[1][i] = mds(1, q41(by));
			pkey->mk_tab[2][i] = mds(2, q42(by));
			pkey->mk_tab[3][i] = mds(3, q43(by));

		}
	}
}




/* The (12,8) Reed Soloman code has the generator polynomial

   g(x) = x^4 + (a + 1/a) * x^3 + a * x^2 + (a + 1/a) * x + 1

   where the coefficients are in the finite field GF(2^8) with a
   modular polynomial a^8 + a^6 + a^3 + a^2 + 1. To generate the
   remainder we have to start with a 12th order polynomial with our
   eight input bytes as the coefficients of the 4th to 11th terms.
   That is:

   m[7] * x^11 + m[6] * x^10 ... + m[0] * x^4 + 0 * x^3 +... + 0

   We then multiply the generator polynomial by m[7] * x^7 and subtract
   it - xor in GF(2^8) - from the above to eliminate the x^7 term (the
   artihmetic on the coefficients is done in GF(2^8). We then multiply
   the generator polynomial by x^6 * coeff(x^10) and use this to remove
   the x^10 term. We carry on in this way until the x^4 term is removed
   so that we are left with:

   r[3] * x^3 + r[2] * x^2 + r[1] 8 x^1 + r[0]

   which give the resulting 4 bytes of the remainder. This is equivalent
   to the matrix multiplication in the Twofish description but much faster
   to implement.

 */

#define G_MOD   0x0000014d

uint mds_rem(uint p0,uint p1)
{
	uint i,t,u;

	for (i = 0; i < 8; ++i) {
		t = p1 >> 24;	/* get most significant coefficient */

		p1 = (p1 << 8) | (p0 >> 24);
		p0 <<= 8;	/* shift others up */

		/* multiply t by a (the primitive element - i.e. left shift) */

		u = (t << 1);

		if (t & 0x80)
			/* subtract modular polynomial on overflow */
			u ^= G_MOD;

		p1 ^= t ^ (u << 16);	/* remove t * (a * x^2 + 1)  */

		u ^= (t >> 1);	/* form u = a * t + t / a = t * (a + 1 / a); */

		if (t & 0x01)
			/* add the modular polynomial on underflow */
			u ^= G_MOD >> 1;

		p1 ^= (u << 24) | (u << 8);	/* remove t * (a + 1/a) * (x^3 + x) */

	}

	return p1;
}

/* initialise the key schedule from the user supplied key   */

void twofish_new_set_key(TWI *pkey,const uint in_key[],const uint key_len)
{
	uint i, a, b, me_key[4], mo_key[4];


	pkey->qt_gen = 0;

	if (!pkey->qt_gen) {
		gen_qtab(pkey);
		pkey->qt_gen = 1;
	}



	pkey->mt_gen = 0;
	if (!pkey->mt_gen) {
		gen_mtab(pkey);
		pkey->mt_gen = 1;
	}


	pkey->k_len = (key_len * 8) / 64;	/* 2, 3 or 4 */

	for (i = 0; i < pkey->k_len; ++i) {

		a = in_key[i + i];
		me_key[i] = a;
		b = in_key[i + i + 1];

		mo_key[i] = b;
		pkey->s_key[pkey->k_len - i - 1] = mds_rem(a, b);
	}

	for (i = 0; i < 40; i += 2) {
		a = 0x01010101 * i;
		b = a + 0x01010101;
		a = h_fun(pkey, a, me_key);
		b = rotl(h_fun(pkey, b, mo_key), 8);
		pkey->l_key[i] = a + b;
		pkey->l_key[i + 1] = rotl(a + 2 * b, 9);
	}

	gen_mk_tab(pkey, (uint *)pkey->s_key);
}

/* decrypt a block of text  */
/*
void my_i_rnd(TWI * pkey, uint * blk , uint i)
{
	uint t0 = 0;
	uint t1 = 0;
	uint t2 = 0;
	uint i_sum = 0;
    uint *key = (uint *)pkey->l_key;
	uint *mk_tab0 = (uint *)&(pkey->mk_tab[0]);
	uint *mk_tab1 = (uint *)&(pkey->mk_tab[1]);
	uint *mk_tab2 = (uint *)&(pkey->mk_tab[2]);
	uint *mk_tab3 = (uint *)&(pkey->mk_tab[3]);
	unsigned char  *buff = (unsigned char *)blk;

	i_sum = i << 2;

	t0 = mk_tab0[buff[0]] ^ mk_tab1[buff[1]] ^ mk_tab2[buff[2]] ^ mk_tab3[buff[3]];
    t1 = mk_tab0[buff[7]] ^ mk_tab1[buff[4]] ^ mk_tab2[buff[5]] ^ mk_tab3[buff[6]];
	t2 = t1 + t0;
	blk[2] = rotl(blk[2], 1) ^ (t2 + key[i_sum + 10]);
    blk[3] = rotr(blk[3] ^ (t2 + t1 + key[i_sum + 11]), 1);

	t0 = mk_tab0[buff[8]] ^ mk_tab1[buff[9]] ^ mk_tab2[buff[10]] ^ mk_tab3[buff[11]];
    t1 = mk_tab0[buff[15]] ^ mk_tab1[buff[12]] ^ mk_tab2[buff[13]] ^ mk_tab3[buff[14]];
	t2 = t1 + t0;
    blk[0] = rotl(blk[0], 1) ^ (t2 + key[i_sum +  8]);
    blk[1] = rotr(blk[1] ^ (t2 + t1 + key[i_sum +  9]), 1);
}
*/

void twofish_new_decrypt(TWI * pkey, uint * in_blk, uint * out_blk)
{
	uint blk[4], i;
	uint *key = (uint *)pkey->l_key;
	uint t0, t1, t2;
	uint i_sum;
	uint *mk_tab0 = (uint *)&(pkey->mk_tab[0]);
	uint *mk_tab1 = (uint *)&(pkey->mk_tab[1]);
	uint *mk_tab2 = (uint *)&(pkey->mk_tab[2]);
	uint *mk_tab3 = (uint *)&(pkey->mk_tab[3]);
	unsigned char  *buff = (unsigned char *)blk;

	blk[0] = in_blk[0] ^ key[4];
	blk[1] = in_blk[1] ^ key[5];
	blk[2] = in_blk[2] ^ key[6];
	blk[3] = in_blk[3] ^ key[7];

    for(i = 7; i <= 7; i--)
    {
	    i_sum = i << 2;

    	t0 = mk_tab0[buff[0]] ^ mk_tab1[buff[1]] ^ mk_tab2[buff[2]] ^ mk_tab3[buff[3]];
        t1 = mk_tab0[buff[7]] ^ mk_tab1[buff[4]] ^ mk_tab2[buff[5]] ^ mk_tab3[buff[6]];
    	t2 = t1 + t0;
    	//blk[2] = rotl(blk[2], 1) ^ (t2 + key[i_sum + 10]);
        //blk[3] = rotr(blk[3] ^ (t2 + t1 + key[i_sum + 11]), 1);
        blk[2] = ((blk[2] << 1) | (blk[2] >> (32 - 1))) ^ (t2 + key[i_sum + 10]);
        blk[3] = (((blk[3] ^ (t2 + t1 + key[i_sum + 11])) >> 1) | ((blk[3] ^ (t2 + t1 + key[i_sum + 11])) << (32 - 1)));

    	t0 = mk_tab0[buff[8]] ^ mk_tab1[buff[9]] ^ mk_tab2[buff[10]] ^ mk_tab3[buff[11]];
        t1 = mk_tab0[buff[15]] ^ mk_tab1[buff[12]] ^ mk_tab2[buff[13]] ^ mk_tab3[buff[14]];
    	t2 = t1 + t0;
        //blk[0] = rotl(blk[0], 1) ^ (t2 + key[i_sum +  8]);
        //blk[1] = rotr(blk[1] ^ (t2 + t1 + key[i_sum +  9]), 1);
        blk[0] = ((blk[0] << 1) | (blk[0] >> (32 - 1))) ^ (t2 + key[i_sum + 8]);
        blk[1] = (((blk[1] ^ (t2 + t1 + key[i_sum + 9])) >> 1) | ((blk[1] ^ (t2 + t1 + key[i_sum + 9])) << (32 - 1)));
    }

	out_blk[0] = blk[2] ^ pkey->l_key[0];
	out_blk[1] = blk[3] ^ pkey->l_key[1];
	out_blk[2] = blk[0] ^ pkey->l_key[2];
	out_blk[3] = blk[1] ^ pkey->l_key[3];

    return;
}

