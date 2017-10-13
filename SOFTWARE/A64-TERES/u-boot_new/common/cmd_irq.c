/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/arch/timer.h>
#include <sprite.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_interrupts(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{

	if (argc != 2)
		return CMD_RET_USAGE;

	/* on */
	if (strncmp(argv[1], "on", 2) == 0)
		enable_interrupts();
	else
		disable_interrupts();

	return 0;
}

U_BOOT_CMD(
	interrupts, 5, 0, do_interrupts,
	"enable or disable interrupts",
	"[on, off]"
);

/* Implemented in $(CPU)/interrupts.c */
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

U_BOOT_CMD(
	irqinfo,    1,    1,     do_irqinfo,
	"print information about IRQs",
	""
);



int do_memcpy_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint  size = 64 * 1024 * 1024;

	tick_printf("memcpy test start\n");
	memcpy((void *)MEMCPY_TEST_DST, (void *)MEMCPY_TEST_SRC, size);
	tick_printf("memcpy test end\n");

	return 0;
}


U_BOOT_CMD(
	memcpy_test, 2, 0, do_memcpy_test,
	"do a memcpy test",
	"NULL"
);


