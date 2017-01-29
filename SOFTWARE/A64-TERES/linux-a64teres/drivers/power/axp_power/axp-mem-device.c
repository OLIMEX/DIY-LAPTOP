#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/regulator/consumer.h>
#include "axp-regu.h"
#include "axp-mem-device.h"

static struct of_device_id axp_mem_device_match[] = {
	{ .compatible = "allwinner,pmu0_regu", },
	{ }
};

static axp_mem_data_t *regulator_count = NULL;
static u32 ldo_count = 0;

static s32 axp_mem_device_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	u32 ldo_index = 0;
	char name[32] = {0};
	s32 num = 0;
	struct axp_consumer_supply consumer_supply[20];
	const char *regulator_string = NULL;

	if (!of_device_is_available(node)) {
		printk(KERN_ERR "%s: axp regu is disable", __func__);
		return -EPERM;
	}

	if (of_property_read_u32(node, "regulator_count", &ldo_count)) {
		printk(KERN_ERR "%s: axp regu get regulator_count failed", __func__);
		return -ENOENT;
	}

	regulator_count = (axp_mem_data_t *)kzalloc(sizeof(axp_mem_data_t)*(ldo_count), GFP_KERNEL);
	if (!regulator_count) {
		printk(KERN_ERR "%s: request regulator_count failed\n", __func__);
		return -1;
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
			strcpy((char*)((regulator_count+ldo_index-1)->id_name),
						consumer_supply[0].supply);
		}

	}
	return 0;
}

int axp_mem_save(void)
{
	u32 ldo_index = 0;
	struct regulator *regu= NULL;
	char * supply_id = NULL;
	char * rtc_id = NULL;
	int ret = 0;

	for (ldo_index = 0; ldo_index < ldo_count; ldo_index++) {
		supply_id = (char*)((regulator_count+ldo_index)->id_name);

		rtc_id = strstr(supply_id, "rtc");
		if (NULL != rtc_id) {
			(regulator_count+ldo_index)->mem_data = 0;
			rtc_id = NULL;
			continue;
		}

		regu= regulator_get(NULL, supply_id);
		if (IS_ERR(regu)) {
			pr_err("%s: some error happen, fail to get regulator %s\n", __func__,
				supply_id);
			return -1;
		}
		ret = regulator_get_voltage(regu);
		if (0 > ret) {
			pr_err("%s: some error happen, fail to get %s voltage!\n", __func__,
				supply_id);
			return -1;
		}
		(regulator_count+ldo_index)->mem_data = ret;

		ret = regulator_is_enabled(regu);
		if (0 < ret) {
			(regulator_count+ldo_index)->mem_data |= (1 << 31);
		} else {
			(regulator_count+ldo_index)->mem_data &=  (~(1 << 31));
		}

		regulator_put(regu);
	}

	return 0;
}
EXPORT_SYMBOL(axp_mem_save);

void axp_mem_restore(void)
{
	u32 ldo_index = 0;
	struct regulator *regu= NULL;
	char * supply_id = NULL;
	int ret = 0, volt_value = 0;

	for (ldo_index = 0; ldo_index < ldo_count; ldo_index++) {
		if (0 == (regulator_count+ldo_index)->mem_data)
			continue;

		supply_id = (char*)((regulator_count+ldo_index)->id_name);
		regu= regulator_get(NULL, supply_id);
		if (IS_ERR(regu)) {
			pr_err("%s: some error happen, fail to get regulator %s\n", __func__,
				supply_id);
		}

		volt_value = (regulator_count+ldo_index)->mem_data & 0x0fffffff;
		ret = regulator_set_voltage(regu, volt_value, volt_value);
		if (0 != ret) {
			pr_err("%s: some error happen, fail to get %s voltage!\n", __func__,
				supply_id);
		}

		if ((regulator_count+ldo_index)->mem_data & (1 << 31)) {
			ret = regu->rdev->desc->ops->enable(regu->rdev);
			if (0 != ret) {
				pr_err("%s: some error happen, fail to enable %s!\n", __func__,
					supply_id);
			}
		} else {
			ret = regu->rdev->desc->ops->disable(regu->rdev);
			if (0 != ret) {
				pr_err("%s: some error happen, fail to enable %s!\n", __func__,
					supply_id);
			}
		}

		regulator_put(regu);
	}

	return;
}
EXPORT_SYMBOL(axp_mem_restore);

static int axp_mem_device_remove(struct platform_device *pdev)
{
	if (NULL != regulator_count) {
		kfree(regulator_count);
		regulator_count = NULL;
	}

	printk(KERN_INFO "%s: module unloaded\n", __func__);

	return 0;
}


static struct platform_driver axp_mem_device_driver = {
	.probe = axp_mem_device_probe,
	.remove = axp_mem_device_remove,
	.driver = {
		.name  = "axp-mem-device",
		.owner = THIS_MODULE,
		.of_match_table = axp_mem_device_match,
	},
};
static int __init axp_mem_device_init(void)
{
	s32 ret;

	ret = platform_driver_register(&axp_mem_device_driver);

	return ret;
}

static void __exit axp_mem_device_exit(void)
{
	platform_driver_unregister(&axp_mem_device_driver);
}


module_init(axp_mem_device_init);
module_exit(axp_mem_device_exit);
MODULE_DESCRIPTION("axp mem driver");
MODULE_AUTHOR("Ming Li<liming@allwinnertech.com>");
MODULE_LICENSE("GPL");

