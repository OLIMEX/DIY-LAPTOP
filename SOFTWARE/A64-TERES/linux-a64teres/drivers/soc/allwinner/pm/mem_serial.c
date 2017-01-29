/**
 * serial.c - common operations
 * date:    2012-2-13 8:42:56
 * author:  Aaron<leafy.myeh@allwinnertech.com>
 * history: V0.1
 */

#include "pm_i.h"

#define	OK		(0)
#define	FAIL		(-1)
#define TRUE		(1)
#define	FALSE		(0)

static __u32 backup_ccu_uart            = 0;
static __u32 backup_ccu_uart_reset      = 0;
static __u32 backup_gpio_uart           = 0;
static __u32 serial_inited_flag = 0;
static __u32 backup_port_id = 0;
#ifdef CONFIG_ARCH_SUN8I
//notice: sun8iw8 use uart2, this interface need support this.
static __u32 set_serial_clk(__u32 mmu_flag)
{
	__u32 			src_freq = 0;
	__u32 			p2clk = 0;

	volatile unsigned int 	*reg = (volatile unsigned int 	*)(0);
	__ccmu_reg_list_t 	*ccu_reg = (__ccmu_reg_list_t 	*)(0);
	__ccmu_apb2_ratio_reg0058_t apb2_reg;

	__u32 port = 0;
	__u32 i = 0;

	if(1 == mmu_flag){
		ccu_reg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);
	}else{
		ccu_reg = (__ccmu_reg_list_t *)(AW_CCM_BASE);
	}
	apb2_reg.dwval = ccu_reg->Apb2Div.dwval;
	//check uart clk src is ok or not.
	//the uart clk src need to be pll6 & clk freq == 600M?
	//so the baudrate == p2clk/(16*div)
	switch(apb2_reg.bits.ClkSrc){
		case 0:
			src_freq = 32000;	//32k
			break;
		case 1:
			src_freq = 24000000;	//24M
			break;
		case 2:
#ifdef CONFIG_ARCH_SUN8IW6P1
			src_freq = 1200000000;	//1200M
#else
			src_freq = 600000000;	//600M
#endif
			break;
		default:
			break;
	}

	//calculate p2clk.
	p2clk = src_freq/((apb2_reg.bits.DivM + 1) * (1<<(apb2_reg.bits.DivN)));

	/*notice:
	**	not all the p2clk is able to create the specified baudrate.
	**	unproper p2clk may result in unacceptable baudrate, just because
	**	the uartdiv is not proper and the baudrate err exceed the acceptable range.
	*/
	if(mmu_flag){
		//backup apb2 gating;
		backup_ccu_uart = *(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA));
		//backup uart reset
		backup_ccu_uart_reset = *(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA));

		//de-assert uart reset
		reg = (volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA));
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));
		change_runtime_env();
		delay_us(1);
		//config uart clk: apb2 gating.
		reg = (volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA));
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));

	}else{
		//de-assert uart reset
		reg = (volatile unsigned int *)(AW_CCU_UART_RESET_PA);
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));
		change_runtime_env();
		delay_us(1);
		//config uart clk
		reg = (volatile unsigned int *)(AW_CCU_UART_PA);
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));

	}

	return p2clk;

}
#endif

#ifdef CONFIG_ARCH_SUN9IW1
static __u32 set_serial_clk(__u32 mmu_flag)
{
	__u32 			src_freq = 0;
	__u32 			p2clk = 0;
	volatile unsigned int 	*reg = (volatile unsigned int 	*)(0);
	__ccmu_reg_list_t 	*ccu_reg = (__ccmu_reg_list_t 	*)(0);
	__u32 port = 0;
	__u32 i = 0;

	ccu_reg = (__ccmu_reg_list_t   *)mem_get_ba();
	//check uart clk src is ok or not.
	//the uart clk src need to be pll6 & clk freq == 600M?
	//so the baudrate == p2clk/(16*div)
	switch(ccu_reg->Apb1_Cfg.bits.apb1_clk_src_sel){
		case 0:
			src_freq = 24000000;	//24M
			break;
		case 1:
			src_freq = 960000000;	//FIXME: need confirm the freq!
			break;
		default:
			break;
	}

	//calculate p2clk.
	p2clk = src_freq/((ccu_reg->Apb1_Cfg.bits.clk_rat_m + 1) * (1<<(ccu_reg->Apb1_Cfg.bits.clk_rat_n)));

	/*notice:
	**	not all the p2clk is able to create the specified baudrate.
	**	unproper p2clk may result in unacceptable baudrate, just because
	**	the uartdiv is not proper and the baudrate err exceed the acceptable range.
	*/
	if(mmu_flag){
		//backup apb2 gating;
		backup_ccu_uart = *(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA));
		//backup uart reset
		backup_ccu_uart_reset = *(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA));

		//config uart clk: apb2 gating.
		reg = (volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA));
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));
		change_runtime_env();
		delay_us(1);
		//de-assert uart reset
		reg = (volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA));
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));

	}else{
		//config uart clk
		reg = (volatile unsigned int *)(AW_CCU_UART_PA);
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));
		change_runtime_env();
		delay_us(1);
		//de-assert uart reset
		reg = (volatile unsigned int *)(AW_CCU_UART_RESET_PA);
		*reg &= ~(1 << (16 + port));
		for( i = 0; i < 100; i++ );
		*reg |=  (1 << (16 + port));

	}

	return p2clk;

}

#endif

#if defined(CONFIG_ARCH_SUN8IW1P1)	//use PF
static void set_serial_gpio(__u32 mmu_flag, __u32 port_id)
{
	__u32 port = 0;
	__u32 i = 0;
	volatile unsigned int 	*reg = (volatile unsigned int *)(0);

	// config uart gpio
	// config tx gpio
	//fpga not need care gpio config;

	if(mmu_flag){
		//backup gpio
		backup_gpio_uart = *(volatile unsigned int *)(IO_ADDRESS(AW_UART_GPIO_PA));
		reg = (__u32 *)(IO_ADDRESS(AW_UART_GPIO_PA));
	}else{
		reg = (__u32 *)(AW_UART_GPIO_PA);
	}

	*reg &= ~(0x707 << (8 + port));
	for( i = 0; i < 100; i++ );
	*reg |=  (0x404 << (8 + port));
	return	;
}

void serial_exit(void)
{
	//restore apb2 gating;
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA)) = backup_ccu_uart;
	//restore uart reset
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA)) = backup_ccu_uart_reset;
	//restore gpio
	*(volatile unsigned int *)(IO_ADDRESS(AW_UART_GPIO_PA)) = backup_gpio_uart;

	serial_exit_manager();
	return	;
}

#elif defined(CONFIG_ARCH_SUN9IW1P1) || defined(CONFIG_ARCH_SUN8IW6P1) \
	|| defined(CONFIG_ARCH_SUN8IW8P1) || defined(CONFIG_ARCH_SUN8IW9P1)
/*
 * if   0 != port_id, mean use specific port(PF) for debug.
 * elif 0== port_id, mean use default port(PH) for debug.
 */
static void set_serial_gpio(__u32 mmu_flag, __u32 port_id)
{
	__u32 i = 0;
	volatile unsigned int 	*reg = (volatile unsigned int *)(0);
	__u32 uart_gpio_mask = 0;
	__u32 uart_gpio_config_val = 0;

	//config gpio clk;
	config_gpio_clk(mmu_flag);

	// config uart gpio
	// config tx gpio
	//fpga not need care gpio config;

	if(mmu_flag){
	    //backup gpio
	    backup_gpio_uart = *(volatile unsigned int *)(IO_ADDRESS(AW_UART_PF_GPIO_PA));
	    reg = (volatile unsigned int *)(IO_ADDRESS(AW_UART_PF_GPIO_PA));
	}else{
	    if(likely(port_id)){
		reg = (volatile unsigned int *)(AW_UART_PF_GPIO_PA);
		uart_gpio_mask = AW_UART_PF_CONFIG_VAL_MASK;
		uart_gpio_config_val = AW_UART_PF_CONFIG_VAL;
	    }else{
		reg = (volatile unsigned int *)(AW_UART_PH_GPIO_PA);
		uart_gpio_mask = AW_UART_PH_CONFIG_VAL_MASK;
		uart_gpio_config_val = AW_UART_PH_CONFIG_VAL;
	    }
	}

	//config uart-gpio
	*reg &= ~(uart_gpio_mask);
	asm volatile ("dsb");
	asm volatile ("isb");

	for( i = 0; i < 100; i++ );
	asm volatile ("dsb");
	asm volatile ("isb");

	*reg |=  (uart_gpio_config_val);
	asm volatile ("dsb");
	asm volatile ("isb");

	return	;
}

void serial_exit(void)
{
	//restore apb2 gating;
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA)) = backup_ccu_uart;
	//restore uart reset
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA)) = backup_ccu_uart_reset;
	//restore gpio
	if( 0 == backup_port_id){//PH
	    *(volatile unsigned int *)(IO_ADDRESS(AW_UART_PH_GPIO_PA)) = backup_gpio_uart;
	}else{
	    *(volatile unsigned int *)(IO_ADDRESS(AW_UART_PF_GPIO_PA)) = backup_gpio_uart;
	}

	serial_exit_manager();
	return	;
}

#elif defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN8IW5P1) || defined(CONFIG_ARCH_SUN8IW10P1)
static void set_serial_gpio(__u32 mmu_flag, __u32 port_id)
{
	__u32 port = 0;
	__u32 i = 0;
	volatile unsigned int 	*reg = (volatile unsigned int *)(0);

	// config uart gpio
	// config tx gpio
	//fpga not need care gpio config;

	if(mmu_flag){
		//backup gpio
		backup_gpio_uart = *(volatile unsigned int *)(IO_ADDRESS(AW_UART_GPIO_PA));
		reg = (volatile unsigned int *)(IO_ADDRESS(AW_UART_GPIO_PA));
	}else{
		reg = (volatile unsigned int *)(AW_UART_GPIO_PA);
	}
	*reg &= ~(0x707 << (8 + port));
	for( i = 0; i < 100; i++ );
	*reg |=  (0x303 << (8 + port));

	return	;
}

void serial_exit(void)
{
	//restore apb2 gating;
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_PA)) = backup_ccu_uart;
	//restore uart reset
	*(volatile unsigned int *)(IO_ADDRESS(AW_CCU_UART_RESET_PA)) = backup_ccu_uart_reset;
	//restore gpio
	*(volatile unsigned int *)(IO_ADDRESS(AW_UART_GPIO_PA)) = backup_gpio_uart;

	serial_exit_manager();
	return	;
}
#endif

void serial_init_nommu(__u32 port_id)
{
	__u32 df = 0;
	__u32 lcr = 0;
	__u32 p2clk = 0;

	set_serial_gpio(0, port_id);
	p2clk = set_serial_clk(0);

	/* set baudrate */
	df = (p2clk + (SUART_BAUDRATE<<3))/(SUART_BAUDRATE<<4);
	lcr = readl(SUART_LCR_PA);
	writel(1, SUART_HALT_PA);
	writel(lcr|0x80, SUART_LCR_PA);
	writel(df>>8, SUART_DLH_PA);
	writel(df&0xff, SUART_DLL_PA);
	writel(lcr&(~0x80), SUART_LCR_PA);
	writel(0, SUART_HALT_PA);

	/* set mode, Set Lin Control Register*/
	writel(3, SUART_LCR_PA);
	/* enable fifo */
	writel(0xe1, SUART_FCR_PA);

	serial_init_manager();
	return	;
}

static void serial_put_char_nommu(char c)
{
	while (!(readl(SUART_USR_PA) & 2));
	asm volatile ("dsb");
	asm volatile ("isb");
	writel(c, SUART_THR_PA);
	asm volatile ("dsb");
	asm volatile ("isb");

	return	;
}

static char serial_get_char_nommu(void)
{
	__u32 time = 0xffff;
	while(!(readl(SUART_USR_PA)&0x08) && time--);
	if (!time)
		return 0;
	return readl(SUART_RBR_PA);
}

__s32 serial_puts_nommu(const char *string)
{
	//ASSERT(string != NULL);

	if(0 == serial_inited_flag){
	    return FAIL;
	}

	while(*string != '\0')
	{
		if(*string == '\n')
		{
			// if current character is '\n',
			// insert output with '\r'.
			serial_put_char_nommu('\r');
		}
		serial_put_char_nommu(*string++);
	}

	return OK;
}

__u32 serial_gets_nommu(char* buf, __u32 n)
{
	__u32 i = 0;
	char c = '\0';

	if(0 == serial_inited_flag){
	    return FAIL;
	}

	for (i=0; i<n; i++) {
		c = serial_get_char_nommu();
		if (c == 0)
			break;
		buf[i] = c;
	}
	return i+1;
}

void serial_init_manager(void)
{
	//set init complete flag;
	serial_inited_flag = 1;
	return ;
}

void serial_exit_manager(void)
{
	//clear init complete flag;
	serial_inited_flag = 0;
	return ;
}
void serial_init(__u32 port_id)
{

	__u32 p2clk = 0;
	__u32 df = 0;
	__u32 lcr = 0;
	backup_port_id = port_id;

	set_serial_gpio(1, port_id);
	p2clk = set_serial_clk(1);

	/* set baudrate */
	df = (p2clk + (SUART_BAUDRATE<<3))/(SUART_BAUDRATE<<4);
	lcr = readl(SUART_LCR);
	writel(1, SUART_HALT);
	writel(lcr|0x80, SUART_LCR);
	writel(df>>8, SUART_DLH);
	writel(df&0xff, SUART_DLL);
	writel(lcr&(~0x80), SUART_LCR);
	writel(0, SUART_HALT);

	/* set mode, Set Lin Control Register*/
	writel(3, SUART_LCR);
	/* enable fifo */
	writel(0xe1, SUART_FCR);

	serial_init_manager();
	return	;
}



static void serial_put_char(char c)
{
	while (!(readl(SUART_USR) & 2));
	asm volatile ("dsb");
	asm volatile ("isb");
	writel(c, SUART_THR);
	asm volatile ("dsb");
	asm volatile ("isb");

	return	;
}

static char serial_get_char(void)
{
	__u32 time = 0xffff;
	while(!(readl(SUART_USR)&0x08) && time--);
	if (!time)
		return 0;
	return (char)(readl(SUART_RBR));
}


/*
*********************************************************************************************************
*                                       	PUT A STRING
*
* Description: 	put out a string.
*
* Arguments  : 	string	: the string which we want to put out.
*
* Returns    : 	OK if put out string succeeded, others if failed.
*********************************************************************************************************
*/
__s32 serial_puts(const char *string)
{
	//ASSERT(string != NULL);

	if(0 == serial_inited_flag){
	    return FAIL;
	}

	while(*string != '\0')
	{
		if(*string == '\n')
		{
			// if current character is '\n',
			// insert output with '\r'.
			serial_put_char('\r');
		}
		serial_put_char(*string++);
	}

	return OK;
}


__u32 serial_gets(char* buf, __u32 n)
{
	__u32 i = 0;
	char c = '\0';

	if(0 == serial_inited_flag){
	    return FAIL;
	}

	for (i=0; i<n; i++) {
		c = serial_get_char();
		if (c == 0)
			break;
		buf[i] = c;
	}
	return i+1;
}
