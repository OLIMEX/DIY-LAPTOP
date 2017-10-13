/*
 * Command for accessing DataFlash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>

extern int sunxi_board_shutdown(void);

int do_shutdown(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return sunxi_board_shutdown();
}

U_BOOT_CMD(
	shutdown,	2,	1,	do_shutdown,
	"shutdown the system",
	"power off the power supply"
);
