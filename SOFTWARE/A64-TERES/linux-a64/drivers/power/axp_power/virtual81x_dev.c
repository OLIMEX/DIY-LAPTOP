#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>
#include <linux/power_supply.h>
#include <linux/mfd/axp-mfd.h>
#include <linux/module.h>

#include "axp-cfg.h"

static struct platform_device virt[]={
	{
			.name = "reg-81x-cs-rtc",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_rtc",
			}
	},{
			.name = "reg-81x-cs-aldo1",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_aldo1",
			}
	},{
			.name = "reg-81x-cs-aldo2",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_aldo2",
			}
	},{
			.name = "reg-81x-cs-aldo3",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_aldo3",
			}
	},{
			.name = "reg-81x-cs-dldo1",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dldo1",
			}
	},{
			.name = "reg-81x-cs-dldo2",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dldo2",
			}
	},{
			.name = "reg-81x-cs-dldo3",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dldo3",
			}
	},{
			.name = "reg-81x-cs-dldo4",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dldo4",
			}
	},{
			.name = "reg-81x-cs-eldo1",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_eldo1",
			}
	},{
			.name = "reg-81x-cs-eldo2",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_eldo2",
			}
	},{
			.name = "reg-81x-cs-eldo3",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_eldo3",
			}
	},{
			.name = "reg-81x-cs-fldo1",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_fldo1",
			}
	},{
			.name = "reg-81x-cs-fldo2",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_fldo2",
			}
	},{
			.name = "reg-81x-cs-dcdc1",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc1",
			}
	},{
			.name = "reg-81x-cs-dcdc2",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc2",
			}
	},{
			.name = "reg-81x-cs-dcdc3",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc3",
			}
	},{
			.name = "reg-81x-cs-dcdc4",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc4",
			}
	},{
			.name = "reg-81x-cs-dcdc5",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc5",
			}
	},{
			.name = "reg-81x-cs-dcdc6",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc6",
			}
	},{
			.name = "reg-81x-cs-dcdc7",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_dcdc7",
			}
	},{
			.name = "reg-81x-cs-gpio0ldo",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_gpio0ldo",
			}
	},{
			.name = "reg-81x-cs-gpio1ldo",
			.id = -1,
			.dev		= {
				.platform_data = "axp81x_gpio1ldo",
		}
	},
};

 static s32 __init virtual_init(void)
{
	s32 j,ret;

	for (j = 0; j < ARRAY_SIZE(virt); j++){
		ret =  platform_device_register(&virt[j]);
		if (ret)
			goto creat_devices_failed;
	}
	return ret;

creat_devices_failed:
	while (j--)
		platform_device_register(&virt[j]);
	return ret;

}

module_init(virtual_init);

static void __exit virtual_exit(void)
{
	s32 j;

	for (j = ARRAY_SIZE(virt) - 1; j >= 0; j--){
		platform_device_unregister(&virt[j]);
	}
}
module_exit(virtual_exit);

MODULE_DESCRIPTION("X-POWERS axp regulator test");
MODULE_AUTHOR("Weijin Zhong");
MODULE_LICENSE("GPL");

