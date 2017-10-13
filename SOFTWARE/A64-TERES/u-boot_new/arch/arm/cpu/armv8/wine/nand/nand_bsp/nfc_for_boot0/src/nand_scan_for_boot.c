/*
************************************************************************************************************************
*                                                      eNand
*                                           Nand flash driver scan module
*
*                             Copyright(C), 2008-2009, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : nand_scan.c
*
* Author : Kevin.z
*
* Version : v0.1
*
* Date : 2008.03.27
*
* Description : This file scan the nand flash storage system, analyze the nand flash type
*               and initiate the physical architecture parameters.
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Kevin.z         2008.03.27      0.1          build the file
*
************************************************************************************************************************
*/
#include "../include/nand_drv_cfg.h"
#include "../include/nand_type.h"
#include "../include/nand_physic.h"
#include "../include/nfc.h"



extern  struct __NandStorageInfo_t  NandStorageInfo;
extern __s32 BOOT_NandGetPara(boot_nand_para_t *param, __u32 size);


void _InitNandPhyInfo(boot_nand_para_t *nand_info)
{
    __u32 i;

    NandStorageInfo.ChannelCnt       = nand_info->ChannelCnt       ;
    NandStorageInfo.ChipCnt          = nand_info->ChipCnt          ;
    NandStorageInfo.ChipConnectInfo  = nand_info->ChipConnectInfo  ;
    NandStorageInfo.RbCnt            = nand_info->RbCnt            ;
    NandStorageInfo.RbConnectInfo	 = nand_info->RbConnectInfo	   ;
    NandStorageInfo.RbConnectMode	 = nand_info->RbConnectMode	   ;
    NandStorageInfo.BankCntPerChip   = nand_info->BankCntPerChip   ;
    NandStorageInfo.DieCntPerChip    = nand_info->DieCntPerChip    ;
    NandStorageInfo.PlaneCntPerDie   = nand_info->PlaneCntPerDie   ;
    NandStorageInfo.SectorCntPerPage = nand_info->SectorCntPerPage ;
    NandStorageInfo.PageCntPerPhyBlk = nand_info->PageCntPerPhyBlk ;
    NandStorageInfo.BlkCntPerDie     = nand_info->BlkCntPerDie     ;
    NandStorageInfo.OperationOpt     = nand_info->OperationOpt & (NAND_LSB_PAGE_TYPE 
| NAND_RANDOM | NAND_READ_RETRY);
    NandStorageInfo.FrequencePar     = nand_info->FrequencePar     ;
    NandStorageInfo.EccMode          = nand_info->EccMode          ;
    NandStorageInfo.ValidBlkRatio    = nand_info->ValidBlkRatio    ;
	NandStorageInfo.ReadRetryType    = nand_info->ReadRetryType    ;
	NandStorageInfo.DDRType          = nand_info->DDRType          ;

	for(i=0;i<8;i++)
	    NandStorageInfo.NandChipId[i]  = nand_info->NandChipId[i]    ;

}

/*
************************************************************************************************************************
*                           SEARCH NAND PHYSICAL ARCHITECTURE PARAMETER
*
*Description: Search the nand flash physical architecture parameter from the parameter table
*             by nand chip ID.
*
*Arguments  : pNandID           the pointer to nand flash chip ID;
*             pNandArchiInfo    the pointer to nand flash physical architecture parameter.
*
*Return     : search result;
*               = 0     search successful, find the parameter in the table;
*               < 0     search failed, can't find the parameter in the table.
************************************************************************************************************************
*/
__s32 _CheckNandID(__u8 *pNandID)
{
    __s32 j=0;

    //search the nand architecture parameter from the given manufacture nand table by nand ID
    while(1)
    {
        for(j=0; j<6; j++)
        {
            //0xff is matching all ID value
            if((pNandID[j] != NandStorageInfo.NandChipId[j])&&(NandStorageInfo.NandChipId[j]!=0xff))
            break;
        }

        if(j==6)
        {
           /*6 bytes of the nand chip ID are all matching, search parameter successful*/
            return 0;
        }

        break;
    }

    //search nand architecture parameter failed
    return -1;
}


/*
************************************************************************************************************************
*                           ANALYZE NAND FLASH STORAGE SYSTEM
*
*Description: Analyze nand flash storage system, generate the nand flash physical
*             architecture parameter and connect information.
*
*Arguments  : none
*
*Return     : analyze result;
*               = 0     analyze successful;
*               < 0     analyze failed, can't recognize or some other error.
************************************************************************************************************************
*/
__s32  BOOT_AnalyzeNandSystem(void)
{
    __s32 result;
    __u8  tmpChipID[8];
    boot_nand_para_t nand_info;

    if( BOOT_NandGetPara( &nand_info, sizeof(boot_nand_para_t) ) != 0 ){
		return -1;
	}

    _InitNandPhyInfo(&nand_info);


    //reset the nand flash chip on boot chip select
    result = PHY_ResetChip(BOOT_CHIP_SELECT_NUM);
    result |= PHY_SynchBank(BOOT_CHIP_SELECT_NUM, SYNC_CHIP_MODE);
    if(result)
      return -1;

   //read nand flash chip ID from boot chip
    result = PHY_ReadNandId(BOOT_CHIP_SELECT_NUM, tmpChipID);
    if(result)
        return -1;

    //check nand ID
    result = _CheckNandID(tmpChipID);
    if(result)
        return -1;


	/*configure page size*/
	{
		NFC_INIT_INFO nfc_info;
		nfc_info.bus_width = 0x0;
		nfc_info.ce_ctl = 0x0;
		nfc_info.ce_ctl1 = 0x0;
		nfc_info.debug = 0x0;
		nfc_info.pagesize = SECTOR_CNT_OF_SINGLE_PAGE;
		nfc_info.rb_sel = 1;
		nfc_info.serial_access_mode = 1;
		nfc_info.ddr_type = DDR_TYPE;
		NFC_ChangMode(&nfc_info);
		NandIndex = 0;
	}

	PHY_ChangeMode(1);

	if( SUPPORT_READ_RETRY
		&& ( (((READ_RETRY_TYPE>>16)&0xff) < 0x10)
			|| (((READ_RETRY_TYPE>>16)&0xff) == 0x40)
			|| (((READ_RETRY_TYPE>>16)&0xff) == 0x41) ))   //boot0 support hynix, micron readretry
	{
		NFC_ReadRetryInit(READ_RETRY_TYPE);
	    PHY_GetDefaultParam(0);
	}
	else
	{
	    NandStorageInfo.ReadRetryType = 0;
	}

    return 0;
}

__u32 NAND_Getlsbpage_type(void)
{
	//PRINT("%x\n",NandStorageInfo.OperationOpt);
	//PRINT("LSB TYPE:%x\n",(((0xff<<12) & NandStorageInfo.OperationOpt)>>12));
	return (((0xff<<12) & NandStorageInfo.OperationOpt)>>12);
}

__u32 Nand_Is_lsb_page(__u32 page)
{
	__u32 retry_mode;
	__u32 NAND_LSBPAGE_TYPE;
	
	NAND_LSBPAGE_TYPE = NAND_Getlsbpage_type();
	
	if(!NAND_LSBPAGE_TYPE)
	{
		return 1;//every page is lsb page
	}

	if(NAND_LSBPAGE_TYPE == 0x20) //samsung 25nm
	{
		if(page==0)
			return 1;
		if(page==PAGE_CNT_OF_PHY_BLK-1)
			return 0;
		if(page%2==1)
			return 1;
		return 0;
	}

	if((NAND_LSBPAGE_TYPE==0x01))//hynix 26nm,20nm
	{
		if((page==0)||(page==1))
			return 1;
		if((page==PAGE_CNT_OF_PHY_BLK-2)||(page==PAGE_CNT_OF_PHY_BLK-1))
			return 0;
		if((page%4 == 2)||(page%4 == 3))
			return 1;
		return 0;
	}

	if((NAND_LSBPAGE_TYPE==0x02))//hynix 16nm
	{
		if(page==0)
			return 1;
		if(page==PAGE_CNT_OF_PHY_BLK-1)
			return 0;
		if(page%2 == 1)
			return 1;
		return 0;
	}

	if(NAND_LSBPAGE_TYPE==0x10)//toshiba 2xnm 19nm 1ynm
	{
		if(page==0)
			return 1;
		if(page==PAGE_CNT_OF_PHY_BLK-1)
			return 0;
		if(page%2 == 1)
			return 1;
		return 0;	
	}

	if(NAND_LSBPAGE_TYPE==0x40)//micron 20nm 29f64g08cbaba
	{
		if((page==0)||(page==1))
			return 1;
		if((page==PAGE_CNT_OF_PHY_BLK-2)||(page==PAGE_CNT_OF_PHY_BLK-1))
			return 0;
		if((page%4 == 2)||(page%4 == 3))
			return 1;
		return 0;
	}	
	
	if(NAND_LSBPAGE_TYPE==0x41)//micron 20nm 29f32g08cbada
	{
		if((page==2)||(page==3))
			return 1;
		if((page==PAGE_CNT_OF_PHY_BLK-2)||(page==PAGE_CNT_OF_PHY_BLK-1))
			return 1;
		if((page%4 == 0)||(page%4 == 1))
			return 1;
		return 0;
	}		
	
	if(NAND_LSBPAGE_TYPE==0x42)//micron 16nm l95b
	{
		if((page==0)||(page==1)||(page==2)||(page==3)||(page==4)||(page==5)||(page==7)||(page==8)||(page==509))
			return 1;
		if((page==6)||(page==508)||(page==511))
			return 0;
		if((page%4 == 2)||(page%4 == 3))
			return 1;
		return 0;
	}

	if(NAND_LSBPAGE_TYPE==0x50)//intel JS29F64G08ACMF3
	{
		if((page==0)||(page==1))
			return 1;
		if((page==PAGE_CNT_OF_PHY_BLK-2)||(page==PAGE_CNT_OF_PHY_BLK-1))
			return 0;
		if((page%4 == 2)||(page%4 == 3))
			return 1;
		return 0;
	}	


	if(NAND_LSBPAGE_TYPE==0x30)//sandisk 2xnm 19nm 1ynm
	{
		if(page==0)
			return 1;
		if(page==PAGE_CNT_OF_PHY_BLK-1)
			return 0;
		if(page%2 == 1)
			return 1;
		return 0;	
	}

}


__u32 NAND_GetLsbblksize(void)
{	
	__u32 i,count;

	count=0;	

	if(NAND_Getlsbpage_type()==0)
		return NandStorageInfo.PageCntPerPhyBlk*NandStorageInfo.SectorCntPerPage*512;

	for(i=0;i<NandStorageInfo.PageCntPerPhyBlk;i++)
	{
		if(Nand_Is_lsb_page(i))
			count++;
	}
	return count*NandStorageInfo.SectorCntPerPage*512;
}

