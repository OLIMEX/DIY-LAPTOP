
//code for iso7816 smart card
#include <linux/module.h>
#include "smartcard.h"
#include "sunxi-scr-common.h"

extern u32 scr_debug_mask;

uint32_t smartcard_params_init(pscatr_struct pscatr)
{
	pscatr->TS = 0x3B;
	pscatr->TK_NUM = 0x00;

	pscatr->T = 0;		//T=0 Protocol
	pscatr->FMAX = 4; //4MHz
	pscatr->F = 372;
	pscatr->D = 1;
	pscatr->I = 50; //50mA
	pscatr->P = 5;  //5V
	pscatr->N = 2;

	return 0;
}

uint32_t smartcard_atr_decode(pscr_struct pscr, pscatr_struct pscatr, uint8_t* pdata, ppps_struct pps, uint32_t with_ts)
{
	uint32_t index=0;
	uint8_t temp;
	uint32_t i;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	pps->ppss = 0xff;  //PPSS
	pps->pps0 = 0;

	if(with_ts)
	{
		pscatr->TS = pdata[0]; //TS
		index ++;
	}
	temp = pdata[index]; //T0
	index ++;
	pscatr->TK_NUM = temp & 0xf;

	if(temp & 0x10) //TA1
	{
		smartcard_ta1_decode(pscatr, pdata[index]);
		pps->pps0 |= 0x1<<4;
		pps->pps1 = pdata[index];
		index ++;
	}
	if(temp & 0x20) //TB1
	{
		smartcard_tb1_decode(pscatr, pdata[index]);
		index++;
	}
	if(temp & 0x40) //TC1
	{
		pscatr->N = pdata[index] & 0xff;
		index ++;
	}
	if(temp & 0x80) //TD1
	{
		dprintk(DEBUG_INIT, "%s: TD1 parse 0x%x !!\n", __func__, pdata[index]);

		temp = pdata[index];
		pscatr->T = temp & 0xf;
		pps->pps0 |= temp & 0xf;
		if(pscatr->T == 1)
				scr_set_t_protocol(pscr, 1);
			else
				scr_set_t_protocol(pscr, 0);
		if(pscatr->N == 0xff)  //Adjust Guard Time
		{
			if(pscatr->T == 1)
				pscatr->N = 1;
			else
				pscatr->N = 2;
		}
		index ++;
	}
	else
	{
		if(pscatr->N == 0xff) pscatr->N = 2;
		goto rx_tk;
	}

	if(temp & 0x10)  //TA2
	{
		dprintk(DEBUG_INIT, "TA2 Exist!!\n");
		index ++;
	}
	if(temp & 0x20)  //TB2
	{
		dprintk(DEBUG_INIT, "TB2 Exist!!\n");
		index ++;
	}
	if(temp & 0x40)  //TC2
	{
		dprintk(DEBUG_INIT, "TC2 Exist!!\n");
		index ++;
	}
	if(temp & 0x80)  //TD2
	{
		dprintk(DEBUG_INIT, "TD2 Exist!!\n");
		temp = pdata[index];
		index ++;
	}
	else
	{
		goto rx_tk;
	}

	if(temp & 0x10)  //TA3
	{
		dprintk(DEBUG_INIT, "TA3 Exist!!\n");
		index ++;
	}
	if(temp & 0x20)  //TB3
	{
		dprintk(DEBUG_INIT, "TB3 Exist!!\n");
		index ++;
	}
	if(temp & 0x40)  //TC3
	{
		dprintk(DEBUG_INIT, "TC3 Exist!!\n");
		index ++;
	}
	if(temp & 0x80)  //TD3
	{
		dprintk(DEBUG_INIT, "TD3 Exist!!\n");
		temp = pdata[index];
		index ++;
	}
	else
	{
		goto rx_tk;
	}

	if(temp & 0x10)  //TA4
	{
		dprintk(DEBUG_INIT, "TA4 Exist!!\n");
		index ++;
	}
	if(temp & 0x20)  //TB4
	{
		dprintk(DEBUG_INIT, "TB4 Exist!!\n");
		index ++;
	}
	if(temp & 0x40)  //TC4
	{
		dprintk(DEBUG_INIT, "TC4 Exist!!\n");
		index ++;
	}
	if(temp & 0x80)  //TD4
	{
		dprintk(DEBUG_INIT, "TD4 Exist!!\n");
		temp = pdata[index];
		index ++;
	}
	else
	{
		goto rx_tk;
	}

rx_tk:
	for(i=0; i<(pscatr->TK_NUM); i++)
	{
		pscatr->TK[i] = pdata[index++];
	}

	pps->pck = pps->ppss;
	pps->pck ^= pps->pps0;
	if(pps->pps0&(0x1<<4))
	{
		pps->pck ^= pps->pps1;
	}
	if(pps->pps0&(0x1<<5))
	{
		pps->pck ^= pps->pps2;
	}
	if(pps->pps0&(0x1<<6))
	{
		pps->pck ^= pps->pps3;
	}

	return 0;
}


void smartcard_ta1_decode(pscatr_struct pscatr, uint8_t ta1)
{
	uint8_t temp = ta1;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	switch((temp>>4)&0xf)
	{
		case 0x0:
			pscatr->FMAX = 4;
			pscatr->F = 372;
			break;
		case 0x1:
			pscatr->FMAX = 5;
			pscatr->F = 372;
			break;
		case 0x2:
			pscatr->FMAX = 6;
			pscatr->F = 558;
			break;
		case 0x3:
			pscatr->FMAX = 8;
			pscatr->F = 744;
			break;
		case 0x4:
			pscatr->FMAX = 12;
			pscatr->F = 1116;
			break;
		case 0x5:
			pscatr->FMAX = 16;
			pscatr->F = 1488;
			break;
		case 0x6:
			pscatr->FMAX = 20;
			pscatr->F = 1860;
			break;
		case 0x9:
			pscatr->FMAX = 5;
			pscatr->F = 512;
			break;
		case 0xA:
			pscatr->FMAX = 7;
			pscatr->F = 768;
			break;
		case 0xB:
			pscatr->FMAX = 10;
			pscatr->F = 1024;
			break;
		case 0xC:
			pscatr->FMAX = 15;
			pscatr->F = 1536;
			break;
		case 0xD:
			pscatr->FMAX = 20;
			pscatr->F = 2048;
			break;
		default:  //0x7/0x8/0xE/0xF
			pscatr->FMAX = 4;
			pscatr->F = 372;
			dprintk(DEBUG_INIT, "Unsupport ta1 = 0x%x\n", ta1);
			break;
	}

	switch(temp&0xf)
	{
		case 0x1:
			pscatr->D = 1;
			break;
		case 0x2:
			pscatr->D = 2;
			break;
		case 0x3:
			pscatr->D = 4;
			break;
		case 0x4:
			pscatr->D = 8;
			break;
		case 0x5:
			pscatr->D = 16;
			break;
		case 0x6:
			pscatr->D = 32;
			break;
		case 0x8:
			pscatr->D = 12;
			break;
		case 0x9:
			pscatr->D = 20;
			break;
		default: //0x0/0x7/0xA/0xB/0xC/0xD/0xE/0xF
			pscatr->D = 1;
			dprintk(DEBUG_INIT, "Unsupport ta1 = 0x%x\n", ta1);
			break;
	}
}

void smartcard_tb1_decode(pscatr_struct pscatr, uint8_t tb1)
{
	uint8_t temp = tb1;

	dprintk(DEBUG_INIT, "%s: enter!!\n", __func__);

	switch((temp>>5)&0x3)
	{
		case 0:
			pscatr->I = 25;
			break;
		case 1:
			pscatr->I = 50;
			break;
		case 2:
			pscatr->I = 100;
			break;
		default:
			pscatr->I = 50;
	}

	if(((temp&0x1f)>4)&&((temp&0x1f)<26))
	{
		pscatr->P = (temp&0x1f); //5~25 in Volts
	}
	else if((temp&0x1f)==0)
	{
		pscatr->P = 0;  //NC
	}
	else
	{
		pscatr->P = 5;  //NC
	}
}

