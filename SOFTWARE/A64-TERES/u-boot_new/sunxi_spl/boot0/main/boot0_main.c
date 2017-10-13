/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include <asm/arch/uart.h>
#include <asm/arch/dram.h>
#include <asm/arch/rtc_region.h>


extern const boot0_file_head_t  BT0_head;

static void print_version(void);
static int boot0_clear_env(void);

extern __s32 boot_set_gpio(void  *user_gpio_list, __u32 group_count_max, __s32 set_gpio);
extern void mmu_setup(u32 dram_size);
extern void  mmu_turn_off( void );
extern int load_boot1(void);
extern void set_dram_para(void *dram_addr , __u32 dram_size, __u32 boot_cpu);
extern void boot0_jump(unsigned int addr);
extern void boot0_jmp_boot1(unsigned int addr);
extern void boot0_jmp_other(unsigned int addr);
extern void boot0_jmp_monitor(void);
extern void reset_pll( void );
extern int load_fip(int *use_monitor);
extern void set_debugmode_flag(void);
extern void set_pll( void );
extern char boot0_hash_value[64];

void __attribute__((weak)) bias_calibration(void)
{
	return;
}

/*******************************************************************************
main:   body for c runtime 
*******************************************************************************/
void main( void )
{
	__u32 status;
	__s32 dram_size;
	__u32 fel_flag;
	__u32 boot_cpu=0;
	int use_monitor = 0;
	//struct spare_boot_head_t* uboot_head=NULL;

	bias_calibration();

	timer_init();
	sunxi_serial_init( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6 );
	set_debugmode_flag();
	printf("HELLO! BOOT0 is starting!\n");
	printf("boot0 commit : %s \n",boot0_hash_value);
	print_version();

	set_pll();

	//sunxi_serial_init( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6 );
	if( BT0_head.prvt_head.enable_jtag )
	{
		boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 6, 1);
	}

	fel_flag = rtc_region_probe_fel_flag();
	if(fel_flag == SUNXI_RUN_EFEX_FLAG)
	{
		rtc_region_clear_fel_flag();
		printf("eraly jump fel\n");
		goto __boot0_entry_err0;
	}
	//mmu_setup();


#ifdef FPGA_PLATFORM
	dram_size = mctl_init((void *)BT0_head.prvt_head.dram_para);
#else
	dram_size = init_DRAM(0, (void *)BT0_head.prvt_head.dram_para);
#endif
	if(dram_size)
	{
		printf("dram size =%d\n", dram_size);
	}
	else
	{
		printf("initializing SDRAM Fail.\n");
		goto  __boot0_entry_err0;
	}
	mmu_setup(dram_size );
	status = load_boot1();
	if(status == 0 )
	{
		use_monitor = 0;
		status = load_fip(&use_monitor);
	}

	printf("Ready to disable icache.\n");

    // disable instruction cache
	mmu_turn_off( ); 

	if( status == 0 )
	{
		//update dram para before jmp to boot1
		set_dram_para((void *)&BT0_head.prvt_head.dram_para, dram_size, boot_cpu);
		printf("Jump to secend Boot.\n");
                if(use_monitor)
		{
			boot0_jmp_monitor();
		}
		else
		{
			boot0_jmp_boot1(CONFIG_SYS_TEXT_BASE);
		}
	}

__boot0_entry_err0:
	boot0_clear_env();

	boot0_jmp_other(FEL_BASE);
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void print_version()
{
	printf("boot0 version : %s\n", BT0_head.boot_head.platform + 2);

	return;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int boot0_clear_env(void)
{

	reset_pll();
	mmu_turn_off();

	__msdelay(10);
    
	return 0;
}
