/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_key.c
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 15:16
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#include "pm_i.h"

static __mem_key_reg_t  *KeyReg;
static __u32 KeyCtrl, KeyIntc;
//static __u32 KeyInts, KeyData0, KeyData1;

//==============================================================================
// QUERRY KEY FOR WAKE UP SYSTEM FROM MEM
//==============================================================================


/*
*********************************************************************************************************
*                                     INIT KEY FOR MEM
*
* Description: init key for mem.
*
* Arguments  : none
*
* Returns    : EPDK_OK;
*********************************************************************************************************
*/
__s32 mem_key_init(void)
{
    /* set key register base */
    KeyReg = (__mem_key_reg_t *)(IO_ADDRESS(AW_LRADC01_BASE));

    /* backup LRADC registers */
    KeyCtrl = KeyReg->Lradc_Ctrl;
    KeyIntc = KeyReg->Lradc_Intc;
    KeyReg->Lradc_Ctrl = 0;
    //note: mem_mdelay(10);
    KeyReg->Lradc_Ctrl = (0x1<<6)|(0x1<<0);
    KeyReg->Lradc_Intc = (0x1<<1);
    KeyReg->Lradc_Ints = (0x1<<1);

    return 0;
}


/*
*********************************************************************************************************
*                                     EXIT KEY FOR MEM
*
* Description: exit key for mem.
*
* Arguments  : none
*
* Returns    : EPDK_OK;
*********************************************************************************************************
*/
__s32 mem_key_exit(void)
{
    KeyReg->Lradc_Ctrl =  KeyCtrl;
    KeyReg->Lradc_Intc =  KeyIntc;
    return 0;
}
/*
*********************************************************************************************************
*                                     QUERY KEY FOR WAKEUP MEM
*
* Description: query key for wakeup mem.
*
* Arguments  : none
*
* Returns    : result;
*               EPDK_TRUE,      get a key;
*               EPDK_FALSE,     no key;
*********************************************************************************************************
*/
__s32 mem_query_key(void)
{
    if(KeyReg->Lradc_Ints & 0x2)
    {
        KeyReg->Lradc_Ints = 0x2;
        return 0;
    }
    return -1;
}

