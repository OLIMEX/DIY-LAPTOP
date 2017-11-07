
/* kernel/power/userscenelock.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : yanggq
 *  Version : v1.0
 *  Date    : 2014-7-27 09:08
 */

#include <linux/ctype.h>
#include <linux/module.h>
#include "linux/power/aw_pm.h"
#include "pm.h"

unsigned int parse_bitmap_en = 0x0;

unsigned int show_gpio_config(char *s, unsigned int size)
{
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	s = NULL;
    }else{
	end = s + size;	
    }

    uk_printf(s, end - s, "\t SUNXI_BANK_SIZE bit 0x%x \n", SUNXI_BANK_SIZE);
    uk_printf(s, end - s, "\t SUNXI_PA_BASE   bit 0x%x \n", SUNXI_PA_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PB_BASE   bit 0x%x \n", SUNXI_PB_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PC_BASE   bit 0x%x \n", SUNXI_PC_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PD_BASE   bit 0x%x \n", SUNXI_PD_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PE_BASE   bit 0x%x \n", SUNXI_PE_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PF_BASE   bit 0x%x \n", SUNXI_PF_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PG_BASE   bit 0x%x \n", SUNXI_PG_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PH_BASE   bit 0x%x \n", SUNXI_PH_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PI_BASE   bit 0x%x \n", SUNXI_PI_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PJ_BASE   bit 0x%x \n", SUNXI_PJ_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PK_BASE   bit 0x%x \n", SUNXI_PK_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PL_BASE   bit 0x%x \n", SUNXI_PL_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PM_BASE   bit 0x%x \n", SUNXI_PM_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PN_BASE   bit 0x%x \n", SUNXI_PN_BASE  );
    uk_printf(s, end - s, "\t SUNXI_PO_BASE   bit 0x%x \n", SUNXI_PO_BASE  );
    uk_printf(s, end - s, "\t AXP_PIN_BASE    bit 0x%x \n", AXP_PIN_BASE   );

    return (s - start);

}

static unsigned int parse_bitmap(char *s, unsigned int size, unsigned int bitmap)
{
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	if(!(parse_bitmap_en & DEBUG_WAKEUP_GPIO_MAP))
	    return 0;
	s = NULL;
    }else{
	end = s + size;	
    }

    switch(bitmap){
	case 1<<0	: uk_printf(s, end - s, "\t\tport 0. \n"); break;
	case 1<<1	: uk_printf(s, end - s, "\t\tport 1. \n"); break;
	case 1<<2	: uk_printf(s, end - s, "\t\tport 2. \n"); break;
	case 1<<3	: uk_printf(s, end - s, "\t\tport 3. \n"); break;
	case 1<<4	: uk_printf(s, end - s, "\t\tport 4. \n"); break;
	case 1<<5	: uk_printf(s, end - s, "\t\tport 5. \n"); break;
	case 1<<6	: uk_printf(s, end - s, "\t\tport 6. \n"); break;
	case 1<<7	: uk_printf(s, end - s, "\t\tport 7. \n"); break;
	case 1<<8	: uk_printf(s, end - s, "\t\tport 8. \n"); break;
	case 1<<9	: uk_printf(s, end - s, "\t\tport 9. \n"); break;
	case 1<<10	: uk_printf(s, end - s, "\t\tport 10. \n"); break;
	case 1<<11	: uk_printf(s, end - s, "\t\tport 11. \n"); break;
	default:						    break;
    }

    return (s - start);

}

static unsigned int parse_group_bitmap(char *s, unsigned int size, unsigned int group_bitmap)
{
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	if(!(parse_bitmap_en & DEBUG_WAKEUP_GPIO_GROUP_MAP))
	    return 0;
	s = NULL;
    }else{
	end = s + size;	
    }

    switch(group_bitmap){
	case 1<<0	: uk_printf(s, end - s, "\t\tgroup 'A'. \n"); break;
	case 1<<1	: uk_printf(s, end - s, "\t\tgroup 'B'. \n"); break;
	case 1<<2	: uk_printf(s, end - s, "\t\tgroup 'C'. \n"); break;
	case 1<<3	: uk_printf(s, end - s, "\t\tgroup 'D'. \n"); break;
	case 1<<4	: uk_printf(s, end - s, "\t\tgroup 'E'. \n"); break;
	case 1<<5	: uk_printf(s, end - s, "\t\tgroup 'F'. \n"); break;
	case 1<<6	: uk_printf(s, end - s, "\t\tgroup 'G'. \n"); break;
	case 1<<7	: uk_printf(s, end - s, "\t\tgroup 'H'. \n"); break;
	case 1<<8	: uk_printf(s, end - s, "\t\tgroup 'I'. \n"); break;
	case 1<<9	: uk_printf(s, end - s, "\t\tgroup 'J'. \n"); break;
	case 1<<10	: uk_printf(s, end - s, "\t\tgroup 'K'. \n"); break;
	case 1<<11	: uk_printf(s, end - s, "\t\tgroup 'L'. \n"); break;
	default:						    break;
    }
    
    return (s - start);
}

unsigned int parse_wakeup_event(char *s, unsigned int size, unsigned int event, event_cpu_id_e cpu_id)
{
    int i = 0;
    int count = 0;
    int counted = 0;
    unsigned int bit_event = 0;
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	if(!(parse_bitmap_en & DEBUG_WAKEUP_SRC)){
	    return 0;
	}
	s = NULL;
    }else{
	end = s + size;	
    }

    uk_printf(s, end - s, "WAKEUP_SRC is as follow: \n");
    //for cpus parse.
    if(CPUS_ID == cpu_id){
	for(i=0; i<32; i++){
	    bit_event = (1<<i & event);
	    switch(bit_event){
		case 0			    : break;
		case CPU0_WAKEUP_MSGBOX         : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_MSGBOX   ", CPU0_WAKEUP_MSGBOX   ); count++;    break;
		case CPU0_WAKEUP_KEY	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_KEY      ", CPU0_WAKEUP_KEY      ); count++;    break;
		case CPUS_WAKEUP_LOWBATT	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_LOWBATT  ", CPUS_WAKEUP_LOWBATT  ); count++;    break; 
		case CPUS_WAKEUP_USB            : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_USB      ", CPUS_WAKEUP_USB      ); count++;    break; 
		//case CPUS_WAKEUP_AC             : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_AC       ", CPUS_WAKEUP_AC       ); count++;    break; 
		case CPUS_WAKEUP_ASCEND         : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_ASCEND   ", CPUS_WAKEUP_ASCEND   ); count++;    break; 
		case CPUS_WAKEUP_DESCEND        : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_DESCEND  ", CPUS_WAKEUP_DESCEND  ); count++;    break; 
		case CPUS_WAKEUP_SHORT_KEY      : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_SHORT_KEY", CPUS_WAKEUP_SHORT_KEY); count++;    break; 
		case CPUS_WAKEUP_LONG_KEY       : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_LONG_KEY ", CPUS_WAKEUP_LONG_KEY ); count++;    break; 
		case CPUS_WAKEUP_IR             : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_IR       ", CPUS_WAKEUP_IR       ); count++;    break; 
		case CPUS_WAKEUP_ALM0           : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_ALM0     ", CPUS_WAKEUP_ALM0     ); count++;    break; 
		case CPUS_WAKEUP_ALM1           : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_ALM1     ", CPUS_WAKEUP_ALM1     ); count++;    break; 
		case CPUS_WAKEUP_TIMEOUT        : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_TIMEOUT  ", CPUS_WAKEUP_TIMEOUT  ); count++;    break; 
		case CPUS_WAKEUP_GPIO           :
						  uk_printf(s, end - s, "\n%-36s bit 0x%x \t ", "CPUS_WAKEUP_GPIO	", CPUS_WAKEUP_GPIO     ); 
						  uk_printf(s, end - s, "\n\twant to know gpio config & suspended status detail? \n\t\tcat /sys/power/aw_pm/debug_mask for help.\n");
						  count++;    
						  break; 
		case CPUS_WAKEUP_USBMOUSE       : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_USBMOUSE ", CPUS_WAKEUP_USBMOUSE ); count++;    break; 
		case CPUS_WAKEUP_LRADC          : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_LRADC    ", CPUS_WAKEUP_LRADC    ); count++;    break; 
		case CPUS_WAKEUP_CODEC          : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_CODEC    ", CPUS_WAKEUP_CODEC    ); count++;    break; 
		case CPUS_WAKEUP_BAT_TEMP       : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_BAT_TEMP ", CPUS_WAKEUP_BAT_TEMP ); count++;    break; 
		case CPUS_WAKEUP_FULLBATT       : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_FULLBATT ", CPUS_WAKEUP_FULLBATT ); count++;    break; 
		case CPUS_WAKEUP_HMIC           : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_HMIC     ", CPUS_WAKEUP_HMIC     ); count++;    break; 
		case CPUS_WAKEUP_POWER_EXP      : uk_printf(s, end -s, "%-36s bit 0x%x \t ", "CPUS_WAKEUP_POWER_EXP", CPUS_WAKEUP_POWER_EXP); count++;    break;
		default:								break;

	    }
	    if(counted != count && 0 == count%2){
		counted = count;
		uk_printf(s, end-s, "\n");
	    }
	}
    }else if(CPU0_ID == cpu_id){  //for cpu0 wakeup src parse.
	for(i=0; i<32; i++){
	    bit_event = (1<<i & event);
	    switch(bit_event){
		case 0			    : break;
		case CPU0_WAKEUP_MSGBOX     : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_MSGBOX ",   CPU0_WAKEUP_MSGBOX ); count++;    break;
		case CPU0_WAKEUP_KEY	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_KEY	", CPU0_WAKEUP_KEY	); count++;    break;
		case CPU0_WAKEUP_EXINT	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_EXINT	", CPU0_WAKEUP_EXINT	); count++;    break; 
		case CPU0_WAKEUP_IR	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_IR	", CPU0_WAKEUP_IR	); count++;    break; 
		case CPU0_WAKEUP_ALARM	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_ALARM	", CPU0_WAKEUP_ALARM	); count++;    break; 
		case CPU0_WAKEUP_USB	    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_USB	", CPU0_WAKEUP_USB	); count++;    break; 
		case CPU0_WAKEUP_TIMEOUT    : uk_printf(s, end - s, "%-36s bit 0x%x \t ", "CPU0_WAKEUP_TIMEOUT",   CPU0_WAKEUP_TIMEOUT); count++;    break; 
		case CPU0_WAKEUP_PIO	    :                                              
					      uk_printf(s, end - s, "\n%-36s bit 0x%x \t ", "CPU0_WAKEUP_PIO	", CPU0_WAKEUP_PIO     ); 
					      uk_printf(s, end - s, "\n\twant to know gpio config & suspended status detail? \n\t\tcat /sys/power/aw_pm/debug_mask for help.\n");
					      count++;    
					      break; 
		default:		    break;

	    }
	    if(counted != count && 0 == count%2){
		counted = count;
		uk_printf(s, end-s, "\n");
	    }
	}
    }

    uk_printf(s, end-s, "\n");

    return (s - start);
}

unsigned int parse_wakeup_gpio_map(char *s, unsigned int size, unsigned int gpio_map)
{
    int i = 0;
    unsigned int bit_event = 0;
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	if(!(parse_bitmap_en & DEBUG_WAKEUP_GPIO_MAP))
	    return 0;
	s = NULL;
    }else{
	end = s + size;	
    }

    uk_printf(s, end - s, "%s", "WAKEUP_GPIO,for cpus:pl,pm, and axp, is as follow: \n");
    
    for(i=0; i<32; i++){
	bit_event = (1<<i & gpio_map);
	if(0 != bit_event){
	    if(bit_event <= WAKEUP_GPIO_PL(GPIO_PL_MAX_NUM)){
		uk_printf(s, end - s, "\tWAKEUP_GPIO_PL	");
		s += parse_bitmap(s, end - s, bit_event);
	    }
	    else if(bit_event <= WAKEUP_GPIO_PM(GPIO_PM_MAX_NUM)){
		uk_printf(s, end - s, "\tWAKEUP_GPIO_PM	");
		s += parse_bitmap(s, end - s, bit_event>>(GPIO_PL_MAX_NUM + 1));

	    }else if(bit_event <= WAKEUP_GPIO_AXP(GPIO_AXP_MAX_NUM)){
		uk_printf(s, end - s, "\tWAKEUP_GPIO_AXP	");
		s += parse_bitmap(s, end - s, bit_event>>(GPIO_PL_MAX_NUM + 1 + GPIO_PM_MAX_NUM + 1));

	    }else {
		uk_printf(s, end - s, "parse err.\n");
	    }   
	}
    }

    return (s - start);
}

unsigned int parse_wakeup_gpio_group_map(char *s, unsigned int size, unsigned int group_map)
{
    int i = 0;
    unsigned int bit_event = 0;
    char *start = s;
    char *end = NULL;

    if(NULL == s || 0 == size){ 
	//buffer is empty
	if(!(parse_bitmap_en & DEBUG_WAKEUP_GPIO_GROUP_MAP))
	    return 0;
	s = NULL;
    }else{
	end = s + size;	
    }
    uk_printf(s, end - s, "WAKEUP_GPIO,for cpux:pa,pb,pc,pd,.., is as follow: \n");
    for(i=0; i<32; i++){
	bit_event = (1<<i & group_map);
	if(0 != bit_event){
		uk_printf(s, end - s, "\tWAKEUP_GPIO_GROUP: ");
		s += parse_group_bitmap(s, end - s, bit_event);
	}
    }

    return (s - start);
}

module_param_named(parse_bitmap_en, parse_bitmap_en, uint, S_IRUGO | S_IWUSR);

