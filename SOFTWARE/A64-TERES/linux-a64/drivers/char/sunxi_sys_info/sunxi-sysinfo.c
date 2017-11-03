/*
 * Based on drivers/char/sunxi-sysinfo/sunxi-sysinfo.c
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
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sunxi-sid.h>

static ssize_t sys_info_show(struct class *class,
			     struct class_attribute *attr, char *buf)
{
	int i;
	int databuf[4] = {0};
	char tmpbuf[129] = {0};
	size_t size = 0;

	/* platform */
	sunxi_get_platform(tmpbuf, 129);
	size += sprintf(buf + size, "sunxi_platform    : %s\n", tmpbuf);

	/* secure */
	size += sprintf(buf + size, "sunxi_secure      : ");
	if (sunxi_soc_is_secure())
		size += sprintf(buf + size, "%s\n", "secure");
	else
		size += sprintf(buf + size, "%s\n", "normal");

	/* chipid */
	sunxi_get_serial((u8 *)databuf);
	for (i = 0; i < 4; i++)
		sprintf(tmpbuf + i*8, "%08x", databuf[i]);
	tmpbuf[128] = 0;
	size += sprintf(buf + size, "sunxi_chipid      : %s\n", tmpbuf);

	/* chiptype */
	sunxi_get_soc_chipid_str(tmpbuf);
	size += sprintf(buf + size, "sunxi_chiptype    : %s\n", tmpbuf);

	/* socbatch number */
	size += sprintf(buf + size, "sunxi_batchno     : %#x\n",
			sunxi_get_soc_ver()&0x0ffff);

	return size;
}

static struct class_attribute info_class_attrs[] = {
	__ATTR(sys_info, 0644, sys_info_show, NULL),
	__ATTR_NULL,
};

static struct class info_class = {
	.name           = "sunxi_info",
	.owner          = THIS_MODULE,
	.class_attrs    = info_class_attrs,
};

static int __init sunxi_sys_info_init(void)
{
	pr_debug("sunxi get sys info driver init\n");
	return class_register(&info_class);
}

static void __exit sunxi_sys_info_exit(void)
{
	pr_debug("sunxi get sys info driver exit\n");
	class_unregister(&info_class);
}

module_init(sunxi_sys_info_init);
module_exit(sunxi_sys_info_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("xiafeng<xiafeng@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi sys info.");
