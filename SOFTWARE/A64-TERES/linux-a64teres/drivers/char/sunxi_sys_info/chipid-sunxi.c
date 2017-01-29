/*
 * Based on arch/arm64/kernel/chipid-sunxi.c
 *
 * Copyright (C) 2015 Allwinnertech Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>

static void __iomem *sun50i_sid_base;

static unsigned int sunxi_soc_chipid[4];
static unsigned int sunxi_serial[4];
static unsigned int sunxi_soc_ver;
static int sunxi_soc_secure = 0;
static unsigned int sunxi_soc_bin = 0;

/**
 * soc version:
 */
unsigned int sunxi_get_soc_ver(void)
{
	return sunxi_soc_ver;
}
EXPORT_SYMBOL(sunxi_get_soc_ver);

/**
 * soc chipid:
 */
int sunxi_get_soc_chipid(u8 *chipid)
{
	memcpy(chipid, sunxi_soc_chipid, 16);

	return 0;
}
EXPORT_SYMBOL(sunxi_get_soc_chipid);

/**
 * soc chipid serial:
 */
int sunxi_get_serial(u8 *serial)
{
	memcpy(serial, sunxi_serial, 16);

	return 0;
}
EXPORT_SYMBOL(sunxi_get_serial);

/**
 * soc chipid str:
 */
int sunxi_get_soc_chipid_str(char *serial)
{
	size_t size;

	size = sprintf(serial, "%08x", sunxi_soc_chipid[0] & 0x0ff);

	return size;
}
EXPORT_SYMBOL(sunxi_get_soc_chipid_str);

/**
 * soc chipid:
 */
int sunxi_soc_is_secure(void)
{
	return sunxi_soc_secure;
}
EXPORT_SYMBOL(sunxi_soc_is_secure);

/**
 * get sunxi soc bin
 *
 * return: the bin of sunxi soc, like that:
 * 0 : fail
 * 1 : slow
 * 2 : normal
 * 3 : fast
 */
unsigned int sunxi_get_soc_bin(void)
{
	return sunxi_soc_bin;
}
EXPORT_SYMBOL(sunxi_get_soc_bin);

static int sunxi_chipid_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	u32 type = 0;

	if (!dev->of_node) {
		dev_err(dev, "device tree node not found\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	sun50i_sid_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(sun50i_sid_base))
		return PTR_ERR(sun50i_sid_base);

	sunxi_soc_chipid[0] = readl(sun50i_sid_base + 0x200);
	sunxi_soc_chipid[1] = readl(sun50i_sid_base + 0x200 + 0x4);
	sunxi_soc_chipid[2] = readl(sun50i_sid_base + 0x200 + 0x8);
	sunxi_soc_chipid[3] = readl(sun50i_sid_base + 0x200 + 0xc);

	/* soc secure bit */
	sunxi_soc_secure = ((readl(sun50i_sid_base + 0x0f4)) >> 11) & 1;

	/* chip version init */
	sunxi_soc_ver = 0;

	sunxi_serial[0] = sunxi_soc_chipid[3];
	sunxi_serial[1] = sunxi_soc_chipid[2];
	sunxi_serial[2] = (sunxi_soc_chipid[1] >> 16) & 0x0FFFF;

	/* soc bin bit0~9 */
	type = (sunxi_soc_chipid[0] >> 0) & 0x3ff;
	switch (type) {
	case 0b000001:
		sunxi_soc_bin = 1;
		break;
	case 0b000011:
		sunxi_soc_bin = 2;
		break;
	case 0b000111:
		sunxi_soc_bin = 3;
		break;
	default:
		break;
	}
	pr_info("%s,%d: soc bin:%d\n", __func__, __LINE__, sunxi_soc_bin);
	pr_info("chipid-sunxi serial %pm\n", sunxi_serial);

	return 0;
}

static const struct of_device_id sunxi_chipid_dt_match[] = {
	{ .compatible = "sunxi,sun50i-chipid", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_chipid_dt_match);

static struct platform_driver sunxi_chipid_driver = {
	.probe		= sunxi_chipid_probe,
	.driver = {
		.name	= "sunxi-chipid",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(sunxi_chipid_dt_match),
	},
};

static int __init sunxi_chipid_drv_register(void)
{
	return platform_driver_register(&sunxi_chipid_driver);
}
postcore_initcall(sunxi_chipid_drv_register);

MODULE_LICENSE     ("GPL v2");
MODULE_AUTHOR      ("xiafeng<xiafeng@allwinnertech.com>");
MODULE_DESCRIPTION ("sunxi chipid driver");
