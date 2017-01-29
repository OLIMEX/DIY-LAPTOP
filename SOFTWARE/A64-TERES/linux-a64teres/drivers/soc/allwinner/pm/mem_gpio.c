#include "pm_types.h"
#include "pm_i.h"

static void *pio_pbase;
static u32 pio_len = 0;

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_init(void)
{
	u32 *base = 0;

	pm_get_dev_info("pio", 0, &base, &pio_len);
        pio_pbase = base;

	return 0;
}

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_save(struct gpio_state *pgpio_state)
{
	int i=0;

	/*save all the gpio reg*/
	for(i=0; i<(GPIO_REG_LENGTH); i++){
		pgpio_state->gpio_reg_back[i] = *(volatile __u32 *)(IO_ADDRESS(pio_pbase) + i*0x04);
	}
	return 0;
}

/*
*********************************************************************************************************
*                                       MEM gpio INITIALISE
*
* Description: mem gpio initialise.
*
* Arguments  : none.
*
* Returns    : 0/-1;
*********************************************************************************************************
*/
__s32 mem_gpio_restore(struct gpio_state *pgpio_state)
{
	int i=0;

	/*restore all the gpio reg*/
	for(i=0; i<(GPIO_REG_LENGTH); i++){
		 *(volatile __u32 *)(IO_ADDRESS(pio_pbase) + i*0x04) = pgpio_state->gpio_reg_back[i];
	}

#ifdef CONFIG_ARCH_SUN8IW1P1
	/* restore watch-dog registers, to avoid IC's bug */
	*(volatile __u32 *)IO_ADDRESS(0x1C20CD8) = 0;
	*(volatile __u32 *)IO_ADDRESS(0x1C20CF8) = 0;
	*(volatile __u32 *)IO_ADDRESS(0x1C20D18) = 0;
#endif

	return 0;
}

#if defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW6P1) || defined(CONFIG_ARCH_SUN8IW8P1)
void config_gpio_clk(__u32 mmu_flag)
{
    static __u32 gpio_clk_inited = 0;
    __u32 gpio_apb0_gating_reg = 0;
    if(0 == gpio_clk_inited){
	if(mmu_flag){
	    gpio_apb0_gating_reg = (__u32)(IO_ADDRESS(AW_CCM_MOD_BASE + AW_CCM_PIO_BUS_GATE_REG_OFFSET));
	}else{
	    gpio_apb0_gating_reg = (__u32)((AW_CCM_MOD_BASE + AW_CCM_PIO_BUS_GATE_REG_OFFSET));
	}

	//first: release gpio gating,then u can opererate gpio.
	writel(readl(gpio_apb0_gating_reg) | (0x1 << 5), gpio_apb0_gating_reg);
	gpio_clk_inited = 1;
    }

    return;

}
#endif


