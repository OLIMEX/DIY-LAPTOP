/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_gtbus.c
* By      : gq.yang
* Version : v1.0
* Date    : 2012-11-3 20:13
* Descript: interrupt for platform mem
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#include "pm_i.h"

#ifdef CONFIG_ARCH_SUN9IW1P1
/*
*********************************************************************************************************
*                                       MEM gtbus SAVE
*
* Description: mem gtbus save.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gtbus_save(struct gtbus_state *gtbus_state)
{
	int i = 0;
	gtbus_state->gtbus_reg 			= 	(gtbus_reg_list_t *)IO_ADDRESS(SUNXI_GTBUS_PBASE);
	gtbus_state->gtbus_reg_backup[0 ]	= 	gtbus_state->gtbus_reg->master_config_reg[0 ] 		;
	gtbus_state->gtbus_reg_backup[1 ]	= 	gtbus_state->gtbus_reg->master_config_reg[1 ] 		;
	gtbus_state->gtbus_reg_backup[2 ]	= 	gtbus_state->gtbus_reg->master_config_reg[2 ] 		;
	gtbus_state->gtbus_reg_backup[3 ]	= 	gtbus_state->gtbus_reg->master_config_reg[3 ] 		;
	gtbus_state->gtbus_reg_backup[4 ]	= 	gtbus_state->gtbus_reg->master_config_reg[4 ] 		;
	gtbus_state->gtbus_reg_backup[5 ]	= 	gtbus_state->gtbus_reg->master_config_reg[5 ] 		;
	gtbus_state->gtbus_reg_backup[6 ]	= 	gtbus_state->gtbus_reg->master_config_reg[6 ] 		;
	gtbus_state->gtbus_reg_backup[7 ]	= 	gtbus_state->gtbus_reg->master_config_reg[7 ] 		;
	gtbus_state->gtbus_reg_backup[8 ]	= 	gtbus_state->gtbus_reg->master_config_reg[8 ] 		;
	gtbus_state->gtbus_reg_backup[9 ]	= 	gtbus_state->gtbus_reg->master_config_reg[9 ] 		;
	gtbus_state->gtbus_reg_backup[10]	= 	gtbus_state->gtbus_reg->master_config_reg[10] 		;
	gtbus_state->gtbus_reg_backup[11]	= 	gtbus_state->gtbus_reg->master_config_reg[11] 		;
	gtbus_state->gtbus_reg_backup[12]	= 	gtbus_state->gtbus_reg->master_config_reg[12] 		;
	gtbus_state->gtbus_reg_backup[13]	= 	gtbus_state->gtbus_reg->master_config_reg[13] 		;
	gtbus_state->gtbus_reg_backup[14]	= 	gtbus_state->gtbus_reg->master_config_reg[14] 		;
	gtbus_state->gtbus_reg_backup[15]	= 	gtbus_state->gtbus_reg->master_config_reg[15] 		;
	gtbus_state->gtbus_reg_backup[16]	= 	gtbus_state->gtbus_reg->master_config_reg[16] 		;
	gtbus_state->gtbus_reg_backup[17]	= 	gtbus_state->gtbus_reg->master_config_reg[17] 		;
	gtbus_state->gtbus_reg_backup[18]	= 	gtbus_state->gtbus_reg->master_config_reg[18] 		;
	gtbus_state->gtbus_reg_backup[19]	= 	gtbus_state->gtbus_reg->master_config_reg[19] 		;
	gtbus_state->gtbus_reg_backup[20]	= 	gtbus_state->gtbus_reg->master_config_reg[20] 		;
	gtbus_state->gtbus_reg_backup[21]	= 	gtbus_state->gtbus_reg->master_config_reg[21] 		;
	gtbus_state->gtbus_reg_backup[22]	= 	gtbus_state->gtbus_reg->master_config_reg[22] 		;
	gtbus_state->gtbus_reg_backup[23]	= 	gtbus_state->gtbus_reg->master_config_reg[23] 		;
	gtbus_state->gtbus_reg_backup[24]	= 	gtbus_state->gtbus_reg->master_config_reg[24] 		;
	gtbus_state->gtbus_reg_backup[25]	= 	gtbus_state->gtbus_reg->master_config_reg[25] 		;
	gtbus_state->gtbus_reg_backup[26]	= 	gtbus_state->gtbus_reg->master_config_reg[26] 		;
	gtbus_state->gtbus_reg_backup[27]	= 	gtbus_state->gtbus_reg->master_config_reg[27] 		;
	gtbus_state->gtbus_reg_backup[28]	= 	gtbus_state->gtbus_reg->master_config_reg[28] 		;
	gtbus_state->gtbus_reg_backup[29]	= 	gtbus_state->gtbus_reg->master_config_reg[29] 		;
	gtbus_state->gtbus_reg_backup[30]	= 	gtbus_state->gtbus_reg->master_config_reg[30] 		;
	gtbus_state->gtbus_reg_backup[31]	= 	gtbus_state->gtbus_reg->master_config_reg[31] 		;
	gtbus_state->gtbus_reg_backup[32]	= 	gtbus_state->gtbus_reg->master_config_reg[32] 		;
	gtbus_state->gtbus_reg_backup[33]	= 	gtbus_state->gtbus_reg->master_config_reg[33] 		;
	gtbus_state->gtbus_reg_backup[34]	= 	gtbus_state->gtbus_reg->master_config_reg[34] 		;
	gtbus_state->gtbus_reg_backup[35]	= 	gtbus_state->gtbus_reg->master_config_reg[35] 		;
	gtbus_state->gtbus_reg_backup[36]	=       gtbus_state->gtbus_reg->band_win_config_reg 		;
	gtbus_state->gtbus_reg_backup[37]	=       gtbus_state->gtbus_reg->master_rd_pri_config_reg_0 	;
	gtbus_state->gtbus_reg_backup[38]	=       gtbus_state->gtbus_reg->master_rd_pri_config_reg_1 	;
	gtbus_state->gtbus_reg_backup[39]	=	gtbus_state->gtbus_reg->config_reg			;
	gtbus_state->gtbus_reg_backup[40]	=	gtbus_state->gtbus_reg->soft_clk_on_reg			;
	gtbus_state->gtbus_reg_backup[41]	=	gtbus_state->gtbus_reg->soft_clk_off_reg		        ;
	gtbus_state->gtbus_reg_backup[42]	=	gtbus_state->gtbus_reg->pmu_en_reg			;
	gtbus_state->gtbus_reg_backup[43]	=	gtbus_state->gtbus_reg->cci400_config_reg_0		;
	gtbus_state->gtbus_reg_backup[44]	=	gtbus_state->gtbus_reg->cci400_config_reg_1		;
	gtbus_state->gtbus_reg_backup[45]	=	gtbus_state->gtbus_reg->cci400_config_reg_2		;
	gtbus_state->gtbus_reg_backup[46]	=	gtbus_state->gtbus_reg->ram_bist_config			;	
	                         
	if(debug_mask&PM_STANDBY_PRINT_GTBUS_REG){
		for(i=0; i<GTBUS_REG_BACKUP_LENGTH; i++){     
			printk("gtbus_state->gtbus_reg_backup[%d] = 0x%x .\n", i, gtbus_state->gtbus_reg_backup[i]);
		}	
	}	                         
	
	return 0;
}

/*
*********************************************************************************************************
*                                       MEM gtbus restore
*
* Description: mem gtbus restore.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gtbus_restore(struct gtbus_state *gtbus_state)
{
	gtbus_state->gtbus_reg->master_config_reg[0 ] 			= 	gtbus_state->gtbus_reg_backup[0 ];
	gtbus_state->gtbus_reg->master_config_reg[1 ] 			= 	gtbus_state->gtbus_reg_backup[1 ];
	gtbus_state->gtbus_reg->master_config_reg[2 ] 			= 	gtbus_state->gtbus_reg_backup[2 ];
	gtbus_state->gtbus_reg->master_config_reg[3 ] 			= 	gtbus_state->gtbus_reg_backup[3 ];
	gtbus_state->gtbus_reg->master_config_reg[4 ] 			= 	gtbus_state->gtbus_reg_backup[4 ];
	gtbus_state->gtbus_reg->master_config_reg[5 ] 			= 	gtbus_state->gtbus_reg_backup[5 ];
	gtbus_state->gtbus_reg->master_config_reg[6 ] 			= 	gtbus_state->gtbus_reg_backup[6 ];
	gtbus_state->gtbus_reg->master_config_reg[7 ] 			= 	gtbus_state->gtbus_reg_backup[7 ];
	gtbus_state->gtbus_reg->master_config_reg[8 ] 			= 	gtbus_state->gtbus_reg_backup[8 ];
	gtbus_state->gtbus_reg->master_config_reg[9 ] 			= 	gtbus_state->gtbus_reg_backup[9 ];
	gtbus_state->gtbus_reg->master_config_reg[10] 			= 	gtbus_state->gtbus_reg_backup[10];
	gtbus_state->gtbus_reg->master_config_reg[11] 			= 	gtbus_state->gtbus_reg_backup[11];
	gtbus_state->gtbus_reg->master_config_reg[12] 			= 	gtbus_state->gtbus_reg_backup[12];
	gtbus_state->gtbus_reg->master_config_reg[13] 			= 	gtbus_state->gtbus_reg_backup[13];
	gtbus_state->gtbus_reg->master_config_reg[14] 			= 	gtbus_state->gtbus_reg_backup[14];
	gtbus_state->gtbus_reg->master_config_reg[15] 			= 	gtbus_state->gtbus_reg_backup[15];
	gtbus_state->gtbus_reg->master_config_reg[16] 			= 	gtbus_state->gtbus_reg_backup[16];
	gtbus_state->gtbus_reg->master_config_reg[17] 			= 	gtbus_state->gtbus_reg_backup[17];
	gtbus_state->gtbus_reg->master_config_reg[18] 			= 	gtbus_state->gtbus_reg_backup[18];
	gtbus_state->gtbus_reg->master_config_reg[19] 			= 	gtbus_state->gtbus_reg_backup[19];
	gtbus_state->gtbus_reg->master_config_reg[20] 			= 	gtbus_state->gtbus_reg_backup[20];
	gtbus_state->gtbus_reg->master_config_reg[21] 			= 	gtbus_state->gtbus_reg_backup[21];
	gtbus_state->gtbus_reg->master_config_reg[22] 			= 	gtbus_state->gtbus_reg_backup[22];
	gtbus_state->gtbus_reg->master_config_reg[23] 			= 	gtbus_state->gtbus_reg_backup[23];
	gtbus_state->gtbus_reg->master_config_reg[24] 			= 	gtbus_state->gtbus_reg_backup[24];
	gtbus_state->gtbus_reg->master_config_reg[25] 			= 	gtbus_state->gtbus_reg_backup[25];
	gtbus_state->gtbus_reg->master_config_reg[26] 			= 	gtbus_state->gtbus_reg_backup[26];
	gtbus_state->gtbus_reg->master_config_reg[27] 			= 	gtbus_state->gtbus_reg_backup[27];
	gtbus_state->gtbus_reg->master_config_reg[28] 			= 	gtbus_state->gtbus_reg_backup[28];
	gtbus_state->gtbus_reg->master_config_reg[29] 			= 	gtbus_state->gtbus_reg_backup[29];
	gtbus_state->gtbus_reg->master_config_reg[30] 			= 	gtbus_state->gtbus_reg_backup[30];
	gtbus_state->gtbus_reg->master_config_reg[31] 			= 	gtbus_state->gtbus_reg_backup[31];
	gtbus_state->gtbus_reg->master_config_reg[32] 			= 	gtbus_state->gtbus_reg_backup[32];
	gtbus_state->gtbus_reg->master_config_reg[33] 			= 	gtbus_state->gtbus_reg_backup[33];
	gtbus_state->gtbus_reg->master_config_reg[34] 			= 	gtbus_state->gtbus_reg_backup[34];
	gtbus_state->gtbus_reg->master_config_reg[35] 			= 	gtbus_state->gtbus_reg_backup[35];
	gtbus_state->gtbus_reg->band_win_config_reg 			=       gtbus_state->gtbus_reg_backup[36];
	gtbus_state->gtbus_reg->master_rd_pri_config_reg_0 		=       gtbus_state->gtbus_reg_backup[37];
	gtbus_state->gtbus_reg->master_rd_pri_config_reg_1 		=       gtbus_state->gtbus_reg_backup[38];
	gtbus_state->gtbus_reg->config_reg				=	gtbus_state->gtbus_reg_backup[39];
	gtbus_state->gtbus_reg->soft_clk_on_reg				=	gtbus_state->gtbus_reg_backup[40];
	gtbus_state->gtbus_reg->soft_clk_off_reg		        	=	gtbus_state->gtbus_reg_backup[41];
	gtbus_state->gtbus_reg->pmu_en_reg				=	gtbus_state->gtbus_reg_backup[42];
#if 0
	busy_waiting();
	//clk off: bit19 = 1;
	gtbus_state->gtbus_reg->cci400_config_reg_1			=	0x00080000;
	//restore reg0, reg1
	gtbus_state->gtbus_reg->cci400_config_reg_1			=	(0x00080000 | ( (~0x00080000)&gtbus_state->gtbus_reg_backup[44] ) );
	gtbus_state->gtbus_reg->cci400_config_reg_0			=	gtbus_state->gtbus_reg_backup[43];
	//sec: reset cci400
	gtbus_state->gtbus_reg->cci400_config_reg_0			=	(0x80000000 | gtbus_state->gtbus_reg_backup[43]);
	//make sure the reset bit is in effect?
	change_runtime_env();
	delay_us(1);
	gtbus_state->gtbus_reg->cci400_config_reg_0			=	(gtbus_state->gtbus_reg_backup[43]);
	//clk on: bit19 = 0;
	gtbus_state->gtbus_reg->cci400_config_reg_1			=	gtbus_state->gtbus_reg_backup[44];

	gtbus_state->gtbus_reg->cci400_config_reg_2			=	gtbus_state->gtbus_reg_backup[45];
#endif

	gtbus_state->gtbus_reg->ram_bist_config				=	gtbus_state->gtbus_reg_backup[46];	
	                                                       
	if(debug_mask&PM_STANDBY_PRINT_GTBUS_REG){
		mem_gtbus_save(gtbus_state);
	}
	                                                                                                  
  	return 0;                                                                                 
}       
#endif

