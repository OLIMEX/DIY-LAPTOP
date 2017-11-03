#include <common.h>
#include <sunxi_board.h>

int do_efex (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf ("## jump to efex ...\n");

	sunxi_board_run_fel();

	return 0;
}



U_BOOT_CMD(
	efex,	1,	0,	do_efex,
	"run to efex",
	"no parameters\n"
);