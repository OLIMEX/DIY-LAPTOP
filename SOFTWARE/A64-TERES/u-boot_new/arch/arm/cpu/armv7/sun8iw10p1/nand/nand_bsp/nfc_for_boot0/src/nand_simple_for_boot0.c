/*********************************************************************************************************
*                                                                NAND FLASH DRIVER
*								(c) Copyright 2008, SoftWinners Co,Ld.
*                                          			    All Right Reserved
*file : nand_simple.c
*description : this file creates some physic basic access function based on single plane for boot .
*history :
*	v0.1  2008-03-26 Richard
* v0.2  2009-9-3 penggang modified for 1615
*
*********************************************************************************************************/

#include "../include/nand_drv_cfg.h"
#include "../include/nand_type.h"
#include "../include/nand_simple.h"
#include "../include/nand_physic.h"
#include "../include/nfc.h"

struct __NandStorageInfo_t  NandStorageInfo;
struct __NandPageCachePool_t PageCachePool;

#ifdef CONFIG_ARCH_SUN9IW1P1
_ndfc_dma_desc_t *ndfc_dma_desc = (void *)0;
#endif

__u32 RetryCount[1] = {0};
const __u16 random_seed[128] = {
    //0        1      2       3        4      5        6       7       8       9
	0x2b75, 0x0bd0, 0x5ca3, 0x62d1, 0x1c93, 0x07e9, 0x2162, 0x3a72, 0x0d67, 0x67f9,
    0x1be7, 0x077d, 0x032f, 0x0dac, 0x2716, 0x2436, 0x7922, 0x1510, 0x3860, 0x5287,
    0x480f, 0x4252, 0x1789, 0x5a2d, 0x2a49, 0x5e10, 0x437f, 0x4b4e, 0x2f45, 0x216e,
    0x5cb7, 0x7130, 0x2a3f, 0x60e4, 0x4dc9, 0x0ef0, 0x0f52, 0x1bb9, 0x6211, 0x7a56,
    0x226d, 0x4ea7, 0x6f36, 0x3692, 0x38bf, 0x0c62, 0x05eb, 0x4c55, 0x60f4, 0x728c,
    0x3b6f, 0x2037, 0x7f69, 0x0936, 0x651a, 0x4ceb, 0x6218, 0x79f3, 0x383f, 0x18d9,
    0x4f05, 0x5c82, 0x2912, 0x6f17, 0x6856, 0x5938, 0x1007, 0x61ab, 0x3e7f, 0x57c2,
    0x542f, 0x4f62, 0x7454, 0x2eac, 0x7739, 0x42d4, 0x2f90, 0x435a, 0x2e52, 0x2064,
    0x637c, 0x66ad, 0x2c90, 0x0bad, 0x759c, 0x0029, 0x0986, 0x7126, 0x1ca7, 0x1605,
    0x386a, 0x27f5, 0x1380, 0x6d75, 0x24c3, 0x0f8e, 0x2b7a, 0x1418, 0x1fd1, 0x7dc1,
    0x2d8e, 0x43af, 0x2267, 0x7da3, 0x4e3d, 0x1338, 0x50db, 0x454d, 0x764d, 0x40a3,
    0x42e6, 0x262b, 0x2d2e, 0x1aea, 0x2e17, 0x173d, 0x3a6e, 0x71bf, 0x25f9, 0x0a5d,
    0x7c57, 0x0fbe, 0x46ce, 0x4939, 0x6b17, 0x37bb, 0x3e91, 0x76db
};

/**************************************************************************
************************* add one cmd to cmd list******************************
****************************************************************************/
void _add_cmd_list(NFC_CMD_LIST *cmd,__u32 value,__u32 addr_cycle,__u8 *addr,__u8 data_fetch_flag,
					__u8 main_data_fetch,__u32 bytecnt,__u8 wait_rb_flag)
{
	cmd->addr = addr;
	cmd->addr_cycle = addr_cycle;
	cmd->data_fetch_flag = data_fetch_flag;
	cmd->main_data_fetch = main_data_fetch;
	cmd->bytecnt = bytecnt;
	cmd->value = value;
	cmd->wait_rb_flag = wait_rb_flag;
	cmd->next = NULL;
}

/****************************************************************************
*********************translate (block + page+ sector) into 5 bytes addr***************
*****************************************************************************/
void _cal_addr_in_chip(__u32 block, __u32 page, __u32 sector,__u8 *addr, __u8 cycle)
{
	__u32 row;
	__u32 column;

	column = 512 * sector;
	row = block * PAGE_CNT_OF_PHY_BLK + page;

	switch(cycle){
		case 1:
			addr[0] = 0x00;
			break;
		case 2:
			addr[0] = column & 0xff;
			addr[1] = (column >> 8) & 0xff;
			break;
		case 3:
			addr[0] = row & 0xff;
			addr[1] = (row >> 8) & 0xff;
			addr[2] = (row >> 16) & 0xff;
			break;
		case 4:
			addr[0] = column && 0xff;
			addr[1] = (column >> 8) & 0xff;
			addr[2] = row & 0xff;
			addr[3] = (row >> 8) & 0xff;
			break;
		case 5:
			addr[0] = column & 0xff;
			addr[1] = (column >> 8) & 0xff;
			addr[2] = row & 0xff;
			addr[3] = (row >> 8) & 0xff;
			addr[4] = (row >> 16) & 0xff;
			break;
		default:
			break;
	}

}

__u8 _cal_real_chip(__u32 global_bank)
{
	return 0;
}

__u8 _cal_real_rb(__u32 chip)
{
	return 0;
}

/*******************************************************************
**********************get status**************************************
********************************************************************/
__s32 _read_status(__u32 cmd_value, __u32 nBank)
{
	/*get status*/
	__u8 addr[5];
	__u32 addr_cycle;
	NFC_CMD_LIST cmd_list;

	addr_cycle = 0;
	_add_cmd_list(&cmd_list, cmd_value, addr_cycle, addr, 1,NFC_IGNORE,1,NFC_IGNORE);

	return (NFC_GetStatus(&cmd_list));

}

/********************************************************************
***************************wait rb ready*******************************
*********************************************************************/
__s32 _wait_rb_ready(__u32 chip)
{
	__s32 timeout = 0xffff;

	/*wait rb ready*/
	while((timeout--) && (NFC_CheckRbReady(0)));
	if (timeout < 0)
		return -ERR_TIMEOUT;

	return 0;
}

void _random_seed_init(void)
{

}

__u32 _cal_random_seed(__u32 page)
{
	__u32 randomseed;

	randomseed = random_seed[page%128];

	return randomseed;
}

__s32 _read_single_page(struct boot_physical_param *readop,__u8 read_mode)
{
	__s32 ret;
	__u32 k = 0;
	__u32 rb;
	__u32 random_seed;
	__u8 sparebuf[4*16];
	__u8 default_value[16];
	__u8 addr[5];
	NFC_CMD_LIST cmd_list[4];
	__u32 list_len,i;


	/*the cammand have no corresponding feature if IGNORE was set, */
	_cal_addr_in_chip(readop->block,readop->page,0,addr,5);
	_add_cmd_list(cmd_list,0x00,5,addr,NFC_NO_DATA_FETCH,NFC_IGNORE,NFC_IGNORE,NFC_NO_WAIT_RB);

	_add_cmd_list(cmd_list + 1,0x05,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE);
	_add_cmd_list(cmd_list + 2,0xe0,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE);
	_add_cmd_list(cmd_list + 3,0x30,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE,NFC_IGNORE);
	list_len = 4;
	for(i = 0; i < list_len - 1; i++){
		cmd_list[i].next = &(cmd_list[i+1]);
	}

	/*wait rb ready*/
	ret = _wait_rb_ready(readop->chip);
	if (ret)
		return ret;

	rb = _cal_real_rb(readop->chip);
	NFC_SelectChip(readop->chip);
	NFC_SelectRb(rb);

    if(SUPPORT_READ_RETRY&&(read_mode == 0))
    {
        if ( ((READ_RETRY_MODE>=0x10)&&(READ_RETRY_MODE<0x30))
		|| ((READ_RETRY_MODE>=0x40)&&(READ_RETRY_MODE<0x50)) )  //toshiba or Samsung mode or micron
            RetryCount[readop->chip] = 0;

        for( k = 0; k<READ_RETRY_CYCLE+1;k++)
		{
			if(RetryCount[readop->chip]==(READ_RETRY_CYCLE+1))
				RetryCount[readop->chip] = 0;

			if(k>0)
			{
			    if(NFC_ReadRetry(readop->chip,RetryCount[readop->chip],READ_RETRY_TYPE))
			    {
			        PHY_ERR("[Read_single_page] NFC_ReadRetry fail \n");
			        return -1;
			    }
			}

			random_seed = _cal_random_seed(readop->page);
			NFC_SetRandomSeed(random_seed);
			NFC_RandomEnable();
			ret = NFC_Read(cmd_list, readop->mainbuf, sparebuf, read_mode , NFC_PAGE_MODE);
			NFC_RandomDisable();

			if((ret != -ERR_ECC)||(k==READ_RETRY_CYCLE))
			{
			    if((READ_RETRY_MODE>=0x10)&&(READ_RETRY_MODE<0x20))  //toshiba mode
			    {
    			    //exit toshiba readretry
    				PHY_ResetChip(readop->chip);
			    }
			    else if((READ_RETRY_MODE>=0x20)&&(READ_RETRY_MODE<0x30))   //samsung mode
			    {
			        NFC_SetDefaultParam(readop->chip, default_value, READ_RETRY_TYPE);
			    }
			    else if((READ_RETRY_MODE>=0x40)&&(READ_RETRY_MODE<0x50))   //micron mode
			    {
			        NFC_SetDefaultParam(readop->chip, default_value, READ_RETRY_TYPE);
			    }

				break;
			}

			RetryCount[readop->chip]++;
		}

    	if(k>0)
    	{
		PHY_DBG("[Read_single_page] NFC_ReadRetry %d cycles, chip = %d, block = %d ", k ,readop->chip,readop->block);
		PHY_DBG("page = %d, RetryCount = %d  \n", readop->page, RetryCount[readop->chip]);

    		if(ret == -ERR_ECC)
    		    PHY_DBG("ecc error!\n");
    		//PHY_DBG("spare buf: %x, %x, %x, %x, %x, %x, %x, %x\n", sparebuf[0],sparebuf[1],sparebuf[2],sparebuf[3],sparebuf[4],sparebuf[5],sparebuf[6],sparebuf[7]);
    	}

    	if(ret == ECC_LIMIT)
    		ret = 0;

    }
    else
    {
		if(SUPPORT_RANDOM)
        {
            if(read_mode == 1)
                random_seed = 0x4a80;
            else
			    random_seed = _cal_random_seed(readop->page);
			NFC_SetRandomSeed(random_seed);
			NFC_RandomEnable();
			ret = NFC_Read(cmd_list, readop->mainbuf, sparebuf, read_mode , NFC_PAGE_MODE);
		}
		else
		{
			ret = NFC_Read(cmd_list, readop->mainbuf, sparebuf, read_mode , NFC_PAGE_MODE);
		}

    }

	if (readop->oobbuf){
		MEMCPY(readop->oobbuf,sparebuf, 2 * 4);
	}

	NFC_DeSelectChip(readop->chip);
	NFC_DeSelectRb(rb);

	return ret;
}



/*
************************************************************************************************************************
*                       INIT NAND FLASH DRIVER PHYSICAL MODULE
*
* Description: init nand flash driver physical module.
*
* Aguments   : none
*
* Returns    : the resutl of initial.
*                   = 0     initial successful;
*                   = -1    initial failed.
************************************************************************************************************************
*/
__s32 PHY_Init(void)
{
    __s32 ret;

	NFC_INIT_INFO nand_info;
    //init RetryCount
    RetryCount[0] = 0;
	nand_info.bus_width = 0x0;
	nand_info.ce_ctl = 0x0;
	nand_info.ce_ctl1 = 0x0;
	nand_info.debug = 0x0;
	nand_info.pagesize = 4;
	nand_info.rb_sel = 1;
	nand_info.serial_access_mode = 1;
	nand_info.ddr_type = 0;
	ret = NFC_Init(&nand_info);

    //PHY_DBG("NFC Randomizer start. \n");
	_random_seed_init();
	NFC_RandomDisable();

	return ret;
}

__s32 PHY_GetDefaultParam(__u32 bank)
{
	__u32 i, j, chip = 0;// rb = 0;
	__u8 default_value[64];
	__u8 oob_buf[64];
	__u8 *oob, *pdata;
	__s32 ret, otp_ok_flag = 0;
	struct boot_physical_param nand_op;

    oob = (__u8 *)(oob_buf);
    pdata = (__u8 *)(PHY_TMP_PAGE_CACHE);
    if (!PageCachePool.PageCache0){
		PageCachePool.PageCache0 = (__u8 *)MALLOC(SECTOR_CNT_OF_SUPER_PAGE * 512);
		if (!PageCachePool.PageCache0)
			return -1;
	}

    if((READ_RETRY_MODE==2)||(READ_RETRY_MODE==3))
    {
        for(i = 8; i<12; i++)
        {
            nand_op.chip = chip;
            nand_op.block = i;
            nand_op.page = 0;
            nand_op.mainbuf = PHY_TMP_PAGE_CACHE;
            nand_op.oobbuf = oob_buf;

            ret = PHY_SimpleRead(&nand_op);

            if((ret>=0)&&(oob[0] == 0x00)&&(oob[1] == 0x4F)&&(oob[2] == 0x4F)&&(oob[3] == 0x42))
            {
                otp_ok_flag = 1;
                for(j=0;j<64;j++)
                {
                    if((pdata[j] + pdata[64+j])!= 0xff)
                    {
                        PHY_DBG("otp data check error!\n");
                        otp_ok_flag = 0;
                        break;
                    }
                }
                if(otp_ok_flag == 1)
                {
                    PHY_DBG("find good otp value in chip %d, block %d \n", nand_op.chip, nand_op.block);
                    break;
                }
            }
        }

        if(otp_ok_flag)
        {
            pdata = (__u8 *)(PHY_TMP_PAGE_CACHE);
            for(j=0;j<64;j++)
                default_value[j] = pdata[j];
            NFC_GetOTPValue(0, default_value, READ_RETRY_TYPE);
        }
        else
        {
            NFC_GetDefaultParam(chip, default_value, READ_RETRY_TYPE);
	        NFC_SetDefaultParam(chip, default_value, READ_RETRY_TYPE);
        }

    }
	else if(READ_RETRY_MODE==4)
    {
			
		otp_ok_flag = 0;
		for(i = 8; i<12; i++)
		{
			nand_op.chip = chip;
			nand_op.block = i;
			nand_op.page = 0;
			nand_op.mainbuf = PHY_TMP_PAGE_CACHE;
			nand_op.oobbuf = oob_buf;

			ret = PHY_SimpleRead(&nand_op);
			
			if((ret>=0)&&(oob[0] == 0x00)&&(oob[1] == 0x4F)&&(oob[2] == 0x4F)&&(oob[3] == 0x42))
			{
				otp_ok_flag = 1;
					
				for(j=0;j<32;j++)
				{
					if((pdata[32+j] + pdata[j])!= 0xff)
					{
						otp_ok_flag = 0;
						break;
					}
				}

				if(otp_ok_flag == 1)
				{
					PHY_DBG("find good otp value in chip %d, block %d \n", nand_op.chip, nand_op.block);
					break;
				}

			}
		}
		if(otp_ok_flag)
		{
			pdata = (__u8 *)(PHY_TMP_PAGE_CACHE);
			NFC_GetOTPValue(chip, pdata, READ_RETRY_TYPE);
		}
		else
		{
			NFC_GetDefaultParam(chip, default_value, READ_RETRY_TYPE);
			NFC_SetDefaultParam(chip, default_value, READ_RETRY_TYPE);
		}			
    }
    else
    {
         NFC_GetDefaultParam(chip, default_value, READ_RETRY_TYPE);
	     NFC_SetDefaultParam(chip, default_value, READ_RETRY_TYPE);
    }

    return 0;
}

__s32 PHY_SetDefaultParam(__u32 bank)
{
	__u8 default_value[64];

	if(SUPPORT_READ_RETRY)
	{
        NFC_SelectChip(0);
        NFC_SetDefaultParam(0, default_value, READ_RETRY_TYPE);
    }
    return 0;
}

__s32 PHY_ChangeMode(__u8 serial_mode)
{

	NFC_INIT_INFO nand_info;

   /*memory allocate*/
	if (!PageCachePool.PageCache0){
		PageCachePool.PageCache0 = (__u8 *)MALLOC(SECTOR_CNT_OF_SUPER_PAGE * 512);
		if (!PageCachePool.PageCache0)
			return -1;
	}

	if (!PageCachePool.SpareCache){
		PageCachePool.SpareCache = (__u8 *)MALLOC(SECTOR_CNT_OF_SUPER_PAGE * 4);
		if (!PageCachePool.SpareCache)
			return -1;
	}

	if (!PageCachePool.TmpPageCache){
		PageCachePool.TmpPageCache = (__u8 *)MALLOC(SECTOR_CNT_OF_SUPER_PAGE * 512);
		if (!PageCachePool.TmpPageCache)
			return -1;
	}
#ifdef  CONFIG_ARCH_SUN9IW1P1
	/*memory allocation for dma descriptor*/
	if (!ndfc_dma_desc) {
		ndfc_dma_desc = (_ndfc_dma_desc_t *)MALLOC(sizeof(_ndfc_dma_desc_t)*5);
		if (!ndfc_dma_desc)
			return -1;
	}
#endif
    NFC_SetEccMode(ECC_MODE);

	nand_info.bus_width = 0x0;
	nand_info.ce_ctl = 0x0;
	nand_info.ce_ctl1 = 0x0;
	nand_info.debug = 0x0;
	nand_info.pagesize = SECTOR_CNT_OF_SINGLE_PAGE;
	nand_info.serial_access_mode = serial_mode;
	nand_info.ddr_type = DDR_TYPE;
	return (NFC_ChangMode(&nand_info));
}


/*
************************************************************************************************************************
*                       NAND FLASH DRIVER PHYSICAL MODULE EXIT
*
* Description: nand flash driver physical module exit.
*
* Aguments   : none
*
* Returns    : the resutl of exit.
*                   = 0     exit successful;
*                   = -1    exit failed.
************************************************************************************************************************
*/
__s32 PHY_Exit(void)
{

	if(SUPPORT_READ_RETRY)
	{
        PHY_SetDefaultParam(0);
        NFC_ReadRetryExit(READ_RETRY_TYPE);
	}

	NFC_RandomDisable();

	NFC_Exit();
	return 0;
}


/*
************************************************************************************************************************
*                       RESET ONE NAND FLASH CHIP
*
*Description: Reset the given nand chip;
*
*Arguments  : nChip     the chip select number, which need be reset.
*
*Return     : the result of chip reset;
*               = 0     reset nand chip successful;
*               = -1    reset nand chip failed.
************************************************************************************************************************
*/
__s32  PHY_ResetChip(__u32 nChip)
{
	__s32 ret;
	__s32 timeout = 0xffff;


	NFC_CMD_LIST cmd;

	NFC_SelectChip(nChip);

	_add_cmd_list(&cmd, 0xff, 0 , NFC_IGNORE, NFC_NO_DATA_FETCH, NFC_IGNORE, NFC_IGNORE, NFC_NO_WAIT_RB);
	ret = NFC_ResetChip(&cmd);

      	/*wait rb0 ready*/
	NFC_SelectRb(0);
	while((timeout--) && (NFC_CheckRbReady(0)));
	if (timeout < 0)
		return -ERR_TIMEOUT;

      /*wait rb0 ready*/
	NFC_SelectRb(1);
	while((timeout--) && (NFC_CheckRbReady(1)));
	if (timeout < 0)
		return -ERR_TIMEOUT;

	NFC_DeSelectChip(nChip);



	return ret;
}


/*
************************************************************************************************************************
*                       READ NAND FLASH ID
*
*Description: Read nand flash ID from the given nand chip.
*
*Arguments  : nChip         the chip number whoes ID need be read;
*             pChipID       the po__s32er to the chip ID buffer.
*
*Return     : read nand chip ID result;
*               = 0     read chip ID successful, the chip ID has been stored in given buffer;
*               = -1    read chip ID failed.
************************************************************************************************************************
*/

__s32  PHY_ReadNandId(__s32 nChip, void *pChipID)
{
	__s32 ret;
	NFC_CMD_LIST cmd;
	__u8 addr = 0;

	NFC_SelectChip(nChip);


	_add_cmd_list(&cmd, 0x90,1 , &addr, NFC_DATA_FETCH, NFC_IGNORE, 6, NFC_NO_WAIT_RB);
	ret = NFC_GetId(&cmd, pChipID);

	NFC_DeSelectChip(nChip);


	return ret;
}


__s32 PHY_SimpleRead (struct boot_physical_param *readop)
{
	return (_read_single_page(readop,0));
}

__s32 PHY_SimpleRead_1K (struct boot_physical_param *readop)
{
	return (_read_single_page(readop,1));
}



/*
************************************************************************************************************************
*                       SYNC NAND FLASH PHYSIC OPERATION
*
*Description: Sync nand flash operation, check nand flash program/erase operation status.
*
*Arguments  : nBank     the number of the bank which need be synchronized;
*             bMode     the type of synch,
*                       = 0     sync the chip which the bank belonged to, wait the whole chip
*                               to be ready, and report status. if the chip support cacheprogram,
*                               need check if the chip is true ready;
*                       = 1     only sync the the bank, wait the bank ready and report the status,
*                               if the chip support cache program, need not check if the cache is
*                               true ready.
*
*Return     : the result of synch;
*               = 0     synch nand flash successful, nand operation ok;
*               = -1    synch nand flash failed.
************************************************************************************************************************
*/
__s32 PHY_SynchBank(__u32 nBank, __u32 bMode)
{
	__s32 ret,status;
	__u32 chip;
	__u32 rb;
	__u32 cmd_value;
	__s32 timeout = 0xffff;

	ret = 0;
	/*get chip no*/
	chip = _cal_real_chip(nBank);
	rb = _cal_real_rb(chip);

	if (0xff == chip){
		PHY_ERR("PHY_SynchBank : beyond chip count\n");
		return -ERR_INVALIDPHYADDR;
	}

	if ( (bMode == 1) && SUPPORT_INT_INTERLEAVE){
		if (nBank%BNK_CNT_OF_CHIP == 0)
			cmd_value = NandStorageInfo.OptPhyOpPar.InterBnk0StatusCmd;
		else
			cmd_value = NandStorageInfo.OptPhyOpPar.InterBnk1StatusCmd;
	}
	else{
		if (SUPPORT_MULTI_PROGRAM)
			cmd_value = NandStorageInfo.OptPhyOpPar.MultiPlaneStatusCmd;
		else
			cmd_value = 0x70;
	}



	NFC_SelectChip(chip);
	NFC_SelectRb(rb);

	while(1){
		status = _read_status(cmd_value,nBank%BNK_CNT_OF_CHIP);
		if (status < 0)
		{
		    PHY_ERR("PHY_SynchBank : read status invalid ,chip = %x, bank = %x, cmd value = %x, status = %x\n",chip,nBank,cmd_value,status);
		    return status;
		}
		if (status & NAND_STATUS_READY)
			break;

		if (timeout-- < 0){
			PHY_ERR("PHY_SynchBank : wait nand ready timeout,chip = %x, bank = %x, cmd value = %x, status = %x\n",chip,nBank,cmd_value,status);
			return -ERR_TIMEOUT;
		}
	}
	if(status & NAND_OPERATE_FAIL)
	{
	    PHY_ERR("PHY_SynchBank : last W/E operation fail,chip = %x, bank = %x, cmd value = %x, status = %x\n",chip,nBank,cmd_value,status);
	    ret = NAND_OP_FALSE;
	}

	NFC_DeSelectChip(chip);
	NFC_DeSelectRb(rb);


	return ret;
}
