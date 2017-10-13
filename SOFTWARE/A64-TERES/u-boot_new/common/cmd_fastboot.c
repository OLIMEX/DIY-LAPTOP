/*
 * Copyright 2008 - 2009 Windriver, <www.windriver.com>
 * Author: Tom Rix <Tom.Rix@windriver.com>
 *
 * (C) Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
//#include <g_dnl.h>

extern int sunxi_usb_dev_register(uint dev_name);
void sunxi_usb_main_loop(int delaytime);

static int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	#if 0
	int ret;

	ret = g_dnl_register("usb_dnl_fastboot");
	if (ret)
		return ret;

	while (1) {
		if (ctrlc())
			break;
		usb_gadget_handle_interrupts();
	}

	g_dnl_unregister();
	return CMD_RET_SUCCESS;
	#endif
	
	if(sunxi_usb_dev_register(3) < 0)
	{
		printf("usb fastboot fail: not support sunxi fastboot\n");

		return -1;
	}

	sunxi_usb_main_loop(0);

	return 0;
}

U_BOOT_CMD(
	fastboot,	1,	1,	do_fastboot,
	"fastboot - enter USB Fastboot protocol",
	""
);
