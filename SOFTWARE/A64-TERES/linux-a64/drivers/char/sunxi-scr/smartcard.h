
//Header file iso7816 smart card

#ifndef _SMARTCARD_H_
#define _SMARTCARD_H_

#include "sunxi-scr.h"

typedef struct{
	uint8_t TS;

	uint8_t TK[15];
	uint8_t TK_NUM;

	uint32_t T;		//Protocol
	uint32_t FMAX; //in MHz
	uint32_t F;
	uint32_t D;
	uint32_t I;   //Max Cunrrent for Program, in mA
	uint32_t P;   //Pogram Voltage
	uint32_t N;   //Extra Guard Time, in ETUs
}scatr_struct, *pscatr_struct;


typedef struct {
	uint8_t ppss;
	uint8_t pps0;
	uint8_t pps1;
	uint8_t pps2;
	uint8_t pps3;
	uint8_t pck;
} upps_struct, *ppps_struct;



uint32_t smartcard_params_init(pscatr_struct pscatr);
uint32_t smartcard_atr_decode(pscr_struct pscr, pscatr_struct pscatr, uint8_t* pdata, ppps_struct pps, uint32_t with_ts);

void smartcard_ta1_decode(pscatr_struct pscatr, uint8_t ta1);
void smartcard_tb1_decode(pscatr_struct pscatr, uint8_t tb1);

#endif          //_SMARTCARD_H_
