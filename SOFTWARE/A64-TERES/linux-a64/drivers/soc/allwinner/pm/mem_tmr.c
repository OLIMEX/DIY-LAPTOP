#include "pm_i.h"

static __mem_tmr_reg_t  *TmrReg;
static __u32 WatchDog1_Config_Reg_Bak, WatchDog1_Mod_Reg_Bak, WatchDog1_Irq_En_Bak;

/*
*********************************************************************************************************
*                                     TIMER save
*
* Description: save timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 mem_tmr_save(__mem_tmr_reg_t *ptmr_state)
{
	/* backup timer registers */
	ptmr_state->IntCtl   = TmrReg->IntCtl;
	ptmr_state->Tmr0Ctl     = TmrReg->Tmr0Ctl;
	ptmr_state->Tmr0IntVal  = TmrReg->Tmr0IntVal;
	ptmr_state->Tmr0CntVal  = TmrReg->Tmr0CntVal;
	ptmr_state->Tmr1Ctl     = TmrReg->Tmr1Ctl;
	ptmr_state->Tmr1IntVal  = TmrReg->Tmr1IntVal;
	ptmr_state->Tmr1CntVal  = TmrReg->Tmr1CntVal;
	
	return 0;
}


/*
*********************************************************************************************************
*                                     TIMER restore
*
* Description: restore timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 mem_tmr_restore(__mem_tmr_reg_t *ptmr_state)
{
	/* restore timer0 parameters */
	TmrReg->Tmr0IntVal  = ptmr_state->Tmr0IntVal;
	TmrReg->Tmr0CntVal  = ptmr_state->Tmr0CntVal;
	TmrReg->Tmr0Ctl     = ptmr_state->Tmr0Ctl;
	TmrReg->Tmr1IntVal  = ptmr_state->Tmr1IntVal;
	TmrReg->Tmr1CntVal  = ptmr_state->Tmr1CntVal;
	TmrReg->Tmr1Ctl     = ptmr_state->Tmr1Ctl;
	TmrReg->IntCtl      = ptmr_state->IntCtl;
	
	return 0;
}

//=================================================use for normal standby ============================
/*
*********************************************************************************************************
*                                     TIMER INIT
*
* Description: initialise timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 mem_tmr_init(void)
{
    u32 *base = 0;
    u32 len = 0;
    pm_get_dev_info("timer", 0, &base, &len);

    /* set timer register base */
    TmrReg = (__mem_tmr_reg_t *)(base);

    WatchDog1_Config_Reg_Bak = (TmrReg->WDog1_Cfg_Reg);
    WatchDog1_Mod_Reg_Bak = (TmrReg->WDog1_Mode_Reg);
    WatchDog1_Irq_En_Bak = (TmrReg->WDog1_Irq_En);

    return 0;
}


/*
*********************************************************************************************************
*                                     TIMER EXIT
*
* Description: exit timer for mem.
*
* Arguments  : none
*
* Returns    : EPDK_TRUE/EPDK_FALSE;
*********************************************************************************************************
*/
__s32 mem_tmr_exit(void)
{
	(TmrReg->WDog1_Cfg_Reg) = WatchDog1_Config_Reg_Bak;
	(TmrReg->WDog1_Mode_Reg) = WatchDog1_Mod_Reg_Bak;
	(TmrReg->WDog1_Irq_En) = WatchDog1_Irq_En_Bak;

	return 0;
}


/*
*********************************************************************************************************
*                           mem_tmr_enable_watchdog
*
*Description: enable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void mem_tmr_enable_watchdog(void)
{

	/* set watch-dog reset to whole system*/ 
	(TmrReg->WDog1_Cfg_Reg) &= ~(0x3);
	(TmrReg->WDog1_Cfg_Reg) |= 0x1;
	/*  timeout is 16 seconds */
	(TmrReg->WDog1_Mode_Reg) = (0xb<<4);
	
	/* enable watch-dog interrupt*/
	(TmrReg->WDog1_Irq_En) |= (1<<0);
	
	/* enable watch-dog */
	(TmrReg->WDog1_Mode_Reg) |= (1<<0);

	return;
}


/*
*********************************************************************************************************
*                           mem_tmr_disable_watchdog
*
*Description: disable watch-dog.
*
*Arguments  : none.
*
*Return     : none;
*
*Notes      :
*
*********************************************************************************************************
*/
void mem_tmr_disable_watchdog(void)
{
	/* disable watch-dog reset: only intterupt */
	(TmrReg->WDog1_Cfg_Reg) &= ~(0x3);
	(TmrReg->WDog1_Cfg_Reg) |= 0x2;
	
	/* disable watch-dog intterupt */
	(TmrReg->WDog1_Irq_En) &= ~(1<<0);
	
	/* disable watch-dog */
	TmrReg->WDog1_Mode_Reg &= ~(1<<0);
}

/*
*********************************************************************************************************
*                           mem_tmr_set
*
*Description: set timer for wakeup system.
*
*Arguments  : second    time value for wakeup system.
*
*Return     : result, 0 - successed, -1 - failed;
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_tmr_set(__u32 second)
{
    /* config timer interrrupt */
    TmrReg->IntSta     = 0x3;
    TmrReg->IntCtl     = 0x3;

#if 0
    /* config timer0 for mem */
    TmrReg->Tmr0Ctl    = 0;
    TmrReg->Tmr0IntVal = second << 10;
    TmrReg->Tmr0Ctl   &= ~(0x3<<2);	    //clk src: 32K
    TmrReg->Tmr0Ctl    = (1<<7) | (5<<4);   //single mode | prescale= /32;
    TmrReg->Tmr0Ctl   |= (1<<1);	    //reload timer 0 interval value;
    TmrReg->Tmr0Ctl   |= (1<<0);	    //start
#else
    /* config timer1 for mem */
    TmrReg->Tmr1Ctl    = 0;
    TmrReg->Tmr1IntVal = second << 10;
    TmrReg->Tmr1Ctl   &= ~(0x3<<2);	    //clk src: 32K
    TmrReg->Tmr1Ctl    = (1<<7) | (5<<4);   //single mode | prescale= /32;
    TmrReg->Tmr1Ctl   |= (1<<1);	    //reload timer 0 interval value;
    TmrReg->Tmr1Ctl   |= (1<<0);	    //start
#endif

    return 0;
}


