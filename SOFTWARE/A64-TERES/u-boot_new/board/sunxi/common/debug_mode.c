

#include <common.h>
#include <sys_config.h>
#include <asm/arch/uart.h>
#include <asm/io.h>
#include <asm/arch/ccmu.h>
#include <power/sunxi/pmu.h>

#define UART_GATE_CTRL  CCMU_BUS_CLK_GATING_REG3
#define UART_RST_CTRL   CCMU_BUS_SOFT_RST_REG4

DECLARE_GLOBAL_DATA_PTR;

#if 0
/*
************************************************************************************************************
*
*                                             function
*
*    name          :	modify_uboot_uart
*
*    parmeters     :
*
*    return        :
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
int modify_uboot_uart(void)
{
	script_gpio_set_t fetch_cfg_gpio[2];
	u32  reg = 0;
	int uart_port_id = 0;
	//disable uart0
	if(script_parser_fetch("uart_para","uart_debug_rx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find card0_rx \n");
		return -1;
	}
	fetch_cfg_gpio[0].mul_sel = 0;
	if(script_parser_patch("uart_para","uart_debug_rx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_rx\n");
		return -1;
	}
	//config uart_tx
	if(script_parser_fetch("uart_para","uart_debug_tx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	fetch_cfg_gpio[1].mul_sel = 0;
	if(script_parser_patch("uart_para","uart_debug_tx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_tx\n");
		return -1;
	}
	//disable uart0
	gpio_request_simple("uart_para",NULL);
	//port_id
	if(script_parser_fetch("force_uart_para","force_uart_port",(int *)(&uart_port_id),sizeof(int)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	if(script_parser_patch("uart_para","uart_debug_port",(int *)(&uart_port_id),sizeof(int)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	if(script_parser_fetch("force_uart_para","force_uart_tx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	if(script_parser_patch("uart_para","uart_debug_tx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_tx\n");
		return -1;
	}
	if(script_parser_fetch("force_uart_para","force_uart_rx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	if(script_parser_patch("uart_para","uart_debug_rx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_tx\n");
		return -1;
	}
	
	printf("uart_port_id = %d\n",uart_port_id);
	uboot_spare_head.boot_data.uart_port = uart_port_id;
	//reset
#ifdef UART_RST_CTRL
	reg = readl(UART_RST_CTRL);
	reg &= ~(1 << (16 + uart_port_id));
	reg |=  (1 << (16 + uart_port_id));
	writel(reg,UART_RST_CTRL);
#endif
	//gate
	reg = readl(UART_GATE_CTRL);
	reg &= ~(1 << (16 + uart_port_id));
	reg |=  (1 << (16 + uart_port_id));
	writel(reg,UART_GATE_CTRL);
	//enable card0
	gpio_request_simple("uart_para",NULL);
	serial_init();
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	modify_system_uart
*
*    parmeters     :
*
*    return        : -1 :fail   0:success
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/

int modify_system_uart(void)
{
	script_gpio_set_t fetch_cfg_gpio[2];
	int uart_port_id = 0;
	char uartname[16] ;
	char uart_data[8] ;
	int sdc0_used = 0,uart_used = 1;
	
	if(script_parser_fetch("force_uart_para","force_uart_rx",(int *)(&fetch_cfg_gpio[0]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find force_uart_rx \n");
		return -1;
	}
	if(script_parser_fetch("force_uart_para","force_uart_tx",(int *)(&fetch_cfg_gpio[1]),sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error: can't find force_uart_tx \n");
		return -1;
	}
	if(script_parser_fetch("force_uart_para","force_uart_port",(int *)(&uart_port_id),sizeof(int)/4))
	{
		printf("debug_mode_error: can't find card0_tx \n");
		return -1;
	}
	memset(uartname,0,16);
	memset(uart_data,0,8);

	strcat(uartname,"uart");

	sprintf(uart_data,"%d",uart_port_id);
	strcat(uartname,uart_data);
	printf("the uartname is %s  \n",uartname);
	if(script_parser_patch(uartname,"uart_used",(int *)(&uart_used),sizeof(int)/4))
	{
		printf("debug_mode_error : can't find patch uart_used\n");
		return -1;
	}
	if(script_parser_patch(uartname,"uart_port",(int *)(&uart_port_id),sizeof(int)/4))
	{
		printf("debug_mode_error : can't find uart_debug_port \n");
		return -1;
	}
	if(script_parser_patch(uartname,"uart_rx",(void*)&fetch_cfg_gpio[0],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_rx\n");
		return -1;
	}
	if(script_parser_patch(uartname,"uart_tx",(void*)&fetch_cfg_gpio[1],sizeof(script_gpio_set_t)/4))
	{
		printf("debug_mode_error : can't patch uart_debug_rx\n");
		return -1;
	}
	//disable card0 init in linux
	if(script_parser_patch("mmc0_para","sdc_used",(int *)(&sdc0_used),sizeof(int)/4))
	{
		printf("debug_mode_error :can not patch sdc_used \n");
		return -1;
	}
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :	change_to_debug_mode
*
*    parmeters     :
*
*    return        : -1 :fail   0:success
*
*    note          :	guoyingyang@allwinnertech.com
*
*
************************************************************************************************************
*/
int debug_mode_set(void)
{
	printf("enter debug mode\n");
	if(modify_uboot_uart())
	{
		printf("debug_mode_error : fail to modify uboot uart\n");
		return -1;
	}
	if(modify_system_uart())
	{
		printf("debug_mode_error: fail to modify system uart\n");
		return -1;
	}
	//if enter debug mode ,set system can print message
	gd->user_debug_mode = 1;
	return 0;
}

int debug_mode_get(void)
{
	return (gd->user_debug_mode == 1) ? 1: 0;
}

void debug_mode_update_info(void)
{
	//if enter debug mode,set loglevel = 8
	char change_env_data[32];
	char *env_concole = "ttyS";
	int baud = 115200;
	int port_id = 0;
	
	if(!debug_mode_get())
		return ;
	memset(change_env_data,0,32);
	sprintf(change_env_data, "%d",8);
	setenv("loglevel",change_env_data);
	if(script_parser_fetch("force_uart_para","force_uart_port",&port_id,sizeof(int)/4))
	{
		printf("card0_print_para port_id fetch error\n");
		return ;
	}
	memset(change_env_data,0,32);
	strcat(change_env_data,"ttyS");
	sprintf(change_env_data,"%s%d%s%d",env_concole,port_id,",",baud);
	setenv("console",change_env_data);

    return ;
}
#else

int debug_mode_get(void)
{
	return  0;
}

int debug_mode_set(void)
{
	return 0;
}

void debug_mode_update_info(void)
{
}

#endif
