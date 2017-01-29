/*
 *
 * Header for semelis
 *
 */

#include <head_data.h>
#include <platform_def.h>

extern char bl31_hash_value[64];

struct spare_boot_ctrl_head  monitor_head __attribute__ ((section(".head_data"))) =
{
		(0x14000000 |(((sizeof(struct spare_boot_ctrl_head)+sizeof(bl31_hash_value)) / sizeof(int)) & 0x00FFFFFF)),
		"monitor",
		0,
		0,
		0,
		0,
		"2.0",
		"monitor",
		{BL31_BASE}
};
