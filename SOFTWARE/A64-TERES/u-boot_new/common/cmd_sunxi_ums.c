#include <common.h>
#include <sunxi_board.h>

extern int sunxi_usb_dev_register(uint dev_name);
void sunxi_usb_main_loop(int delaytime);

int do_sunxi_ums (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("run usb mass\n");
	if(sunxi_usb_dev_register(1))
	{
		printf("sunxi usb mass: invalid usb device\n");
		return -1;
	}
	sunxi_usb_main_loop(0);

	return 0;
}


U_BOOT_CMD(
	sunxi_ums,	1,	0,	do_sunxi_ums,
	"sunxi_ums - present sunxi flash as USB Storage device",
	"no parameters\n"
);
