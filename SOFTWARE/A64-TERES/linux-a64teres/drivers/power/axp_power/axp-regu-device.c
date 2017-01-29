#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/power/axp_depend.h>
#include "axp-regu.h"

static struct of_device_id axp_regu_device_match[] = {
	{ .compatible = "allwinner,", .data = NULL },
	{ }
};

static struct axp_consumer_supply *consumer_supply_count = NULL;

static s32 axp_regu_device_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	struct axp_reg_init *axp_init_data = NULL;
	u32 ldo_count = 0, ldo_index = 0;
	char name[32] = {0};
	s32 num = 0, supply_num = 0, i = 0, j =0, var = 0;
	struct axp_consumer_supply consumer_supply[20];
	const char *regulator_string = NULL;
	struct regulator_consumer_supply *regu_consumer_supply = NULL;

	device = of_match_device(axp_regu_device_match, &pdev->dev);
	if (!device)
		return -ENODEV;
	axp_init_data = (struct axp_reg_init *)device->data;
	if (NULL == axp_init_data) {
		return -ENOENT;
	}

	if (!of_device_is_available(node)) {
		printk(KERN_ERR "%s: axp regu is disable", __func__);
		return -EPERM;
	}

	if (of_property_read_u32(node, "regulator_count", &ldo_count)) {
		printk(KERN_ERR "%s: axp regu get regulator_count failed", __func__);
		return -ENOENT;
	}

	for (ldo_index = 1; ldo_index <= ldo_count; ldo_index++) {
		sprintf(name, "regulator%d", ldo_index);
		if (of_property_read_string(node, (const char *)&name,
					&regulator_string)) {
			printk(KERN_ERR "node %s get failed!\n",
				  name);
			continue;
		}

		num = sscanf(regulator_string, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			consumer_supply[0].supply, consumer_supply[1].supply, consumer_supply[2].supply,
			consumer_supply[3].supply, consumer_supply[4].supply, consumer_supply[5].supply,
			consumer_supply[6].supply, consumer_supply[7].supply, consumer_supply[8].supply,
			consumer_supply[9].supply, consumer_supply[10].supply, consumer_supply[11].supply,
			consumer_supply[12].supply, consumer_supply[13].supply, consumer_supply[14].supply,
			consumer_supply[15].supply, consumer_supply[16].supply, consumer_supply[17].supply,
			consumer_supply[18].supply, consumer_supply[19].supply);

		if (num <= -1) {
			printk(KERN_ERR "parse ldo%d from sysconfig failed\n", ldo_index);
			return -1;
		} else {
			if (strcmp(consumer_supply[1].supply, "none")) {
				var = simple_strtoul(consumer_supply[1].supply, NULL, 10);
				if (var > (ldo_index-1))
					printk(KERN_ERR "supply rely set err\n");
				else
					(*(axp_init_data+(ldo_index-1))).axp_reg_init_data.supply_regulator = ((*(axp_init_data+(var-1))).axp_reg_init_data.consumer_supplies)->supply;
			}
			supply_num = num-1;
			(*(axp_init_data+(ldo_index-1))).axp_reg_init_data.num_consumer_supplies = supply_num;

			consumer_supply_count = (struct axp_consumer_supply *)kzalloc(sizeof(struct axp_consumer_supply)*supply_num, GFP_KERNEL);
			if (!consumer_supply_count) {
				printk(KERN_ERR "%s: request consumer_supply_count failed\n", __func__);
				return -1;
			}

			regu_consumer_supply = (struct regulator_consumer_supply *)kzalloc(sizeof(struct regulator_consumer_supply)*supply_num, GFP_KERNEL);
			if (!regu_consumer_supply) {
				printk(KERN_ERR "%s: request regu_consumer_supply failed\n", __func__);
				kfree(consumer_supply_count);
				return -1;
			}

			for (i = 0; i < supply_num; i++) {
				if (0 != i)
					j = i + 1;
				else
					j = i;
				strcpy((char*)(consumer_supply_count+i),
						consumer_supply[j].supply);
				(regu_consumer_supply+i)->supply = (const char*)((struct axp_consumer_supply *)(consumer_supply_count+i)->supply);

				{
					int ret = 0, sys_id_conut = 0;

					sys_id_conut = axp_check_sys_id((consumer_supply_count+i)->supply);
					if (0 <= sys_id_conut) {
						ret = get_ldo_dependence((const char *)&(consumer_supply[0].supply), sys_id_conut);
						if (ret < 0)
							printk("sys_id %s set dependence failed. \n", (consumer_supply_count+i)->supply);
					}
				}

			}
			(*(axp_init_data+(ldo_index-1))).axp_reg_init_data.consumer_supplies = regu_consumer_supply;
		}

	}
	return 0;
}

static struct platform_driver axp_regu_device_driver = {
	.probe = axp_regu_device_probe,
	.driver = {
		.name  = "axp-regu-device",
		.owner = THIS_MODULE,
		.of_match_table = axp_regu_device_match,
	},
};
s32  axp_regu_device_tree_parse(char * pmu_type, struct axp_reg_init *axp_init_data)
{
	s32 ret;

	strcat(axp_regu_device_match[0].compatible, pmu_type);

	axp_regu_device_match[0].data = (void *)axp_init_data;

	ret = platform_driver_register(&axp_regu_device_driver);

	return ret;
}

