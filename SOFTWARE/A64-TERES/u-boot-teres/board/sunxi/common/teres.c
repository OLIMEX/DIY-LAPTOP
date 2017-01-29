
#include <common.h>
#include <power/sunxi/axp.h>
#include <power/sunxi/pmu.h>
#include <i2c.h>
#include <asm/io.h>
#include <malloc.h>
#include "teres.h"
DECLARE_GLOBAL_DATA_PTR;

int teres_set_axp_voltages(void)
{
	axp_set_supply_status(0, PMU_SUPPLY_DLDO2, 2700, 0);
        axp_set_supply_status(0, PMU_SUPPLY_DLDO3, 1200, 0);
	axp_set_supply_status(0, PMU_SUPPLY_DCDC5, 1350, 1);

        printf("Setting DLDO2 and DLDO3 Voltage\n");
        axp_set_supply_status(0, PMU_SUPPLY_DLDO2, 2700, 1);
        __udelay(1300);
        axp_set_supply_status(0, PMU_SUPPLY_DLDO3, 1200, 1);

	return 0;
}
