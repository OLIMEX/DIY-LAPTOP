
#include <common.h>
#include <fdt_support.h>
#include <sys_config.h>
#include "../cartoon/sprite_cartoon.h"
#include "../cartoon/sprite_cartoon_i.h"
#include "../cartoon/sprite_cartoon_color.h"
#include <sunxi_display2.h>
#include <sunxi_board.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_OLIMEX_MODEL
int has_anx9807_chip(void);
#endif
int sprite_debug(void)
{

        int screen_width, screen_height;
        int x1, x2, y1, y2;

        sprite_cartoon_screen_set();
        board_display_show_until_lcd_open(0);

        screen_width  = borad_display_get_screen_width();
        screen_height = borad_display_get_screen_height();


        x1 = screen_width/4;
        x2 = x1 * 3;

        y1 = screen_height/2 - 40;
        y2 = screen_height/2 + 40;

        printf("bar x1: %d y1: %d\n", x1, y1);
        printf("bar x2: %d y2: %d\n", x2, y2);


        sprite_uichar_init(24);
        sprite_uichar_printf("Debug mode selected !\n");


return 0;
}
int set_misc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
//FDT_PATH_HDEBUG
	
         user_gpio_set_t gpio_init;
        int     ret = 0;
	int 	ret1 = 0;
        int     enabled = 0;
        int nodeoffset;
	__u32  gpio_hd = 0;

        nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_HDEBUG);
        if(nodeoffset >0)
        {
                fdt_getprop_u32(working_fdt, nodeoffset,"enabled", (uint32_t*)&enabled);
		printf("Headphone DEBUG State :%d\n",enabled);
        }

        memset(&gpio_init, 0, sizeof(user_gpio_set_t));

        ret = fdt_get_one_gpio(FDT_PATH_HDEBUG, "debug_en_gpio",&gpio_init);
        if(!ret)
        {
                if(gpio_init.port)
                {
                        gpio_hd = gpio_request(&gpio_init, 1);
                        if(!gpio_hd)
                        {
                                printf("reuqest gpio for headphone debug failed\n");
                                return 1;
                        } else {
			
				 if (enabled == 0 ) {
                        		ret1 =  gpio_write_one_pin_value(gpio_hd, 1, "debug_en_gpio");
                			} 
			         if (enabled == 1 ) {
					 ret1 = gpio_write_one_pin_value(gpio_hd, 0, "debug_en_gpio");
					// sprite_debug();
                       			}		


				}

                       
                }

		 printf("Headphone DEBUG State GPIO:%d\n",ret1);
        }
		//sunxi_bmp_display("bootlogo.bmp");
        return 0;


}
int get_model(char* model)
{

#ifdef CONFIG_OLIMEX_MODEL
	if (has_anx9807_chip()) {
		puts("Teres: has ANX9807 chip\n");
		sprintf(model, "teres");
	} else {
		sprintf(model, "a64-olinuxino");
	}
#endif

	return 0;
}

int do_env_set_debug(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	 user_gpio_set_t gpio_init;
	char debug_flag[3] = {0};
	int enabled = 0;
	int     ret = 0;
        int     ret1 = 0;
        __u32  gpio_hd = 0;
//	int nodeoffset =0;
// nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_HDEBUG);
	strcpy(debug_flag,getenv("debug"));
	printf("Test %s \n",debug_flag);
	if (strcmp(debug_flag,"on") == 0) {
		enabled = 1;

	} 





        memset(&gpio_init, 0, sizeof(user_gpio_set_t));

        ret = fdt_get_one_gpio(FDT_PATH_HDEBUG, "debug_en_gpio",&gpio_init);
        if(!ret)
        {
                if(gpio_init.port)
                {
                        gpio_hd = gpio_request(&gpio_init, 1);
                        if(!gpio_hd)
                        {
                                printf("reuqest gpio for headphone debug failed\n");
                                return 1;
                        } else {

                                 if (enabled == 0 ) {
                                        ret1 =  gpio_write_one_pin_value(gpio_hd, 1, "debug_en_gpio");
                                        }
                                 if (enabled == 1 ) {
                                         ret1 = gpio_write_one_pin_value(gpio_hd, 0, "debug_en_gpio");
                                        // sprite_debug();
                                        }


                                }


                }

                 printf("Headphone DEBUG State GPIO:%d\n",ret1);
        }




return 0;
}
int olimex_set_model(void)
{
	char model[128] = {0}	;
	get_model(model);
	printf("Model: %s\n", model);
	if(setenv("olimex_model", model))
	{
		printf("error:set variable [olimex_model] fail\n");
	}
	//set_misc();
	return 0;
}
U_BOOT_CMD(
        set_debug,    1,      0,      set_misc,
        "get debug info from fdt",
        "no args\n"
);
U_BOOT_CMD(
        env_set_debug,    1,      0,	do_env_set_debug,
        "get debug info from env",
        "no args\n"
);


