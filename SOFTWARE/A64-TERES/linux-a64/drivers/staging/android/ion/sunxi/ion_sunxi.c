/*
 * drivers/gpu/ion/sunxi/sunxi_ion.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi ion heap realization
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <asm/cacheflush.h>
#include "../ion_priv.h"
#include "../../uapi/ion_sunxi.h"
#include "ion_sunxi.h"

struct ion_device;
static struct ion_device *ion_device;
int ion_handle_put(struct ion_handle *handle);
long sunxi_ion_ioctl(struct ion_client *client, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct ion_handle *ion_handle_get_by_id(struct ion_client *client,int id);
	
	switch(cmd) {
	case ION_IOC_SUNXI_PHYS_ADDR:
	{
		sunxi_phys_data data;
		struct ion_handle *handle;
		if(copy_from_user(&data, (void __user *)arg, sizeof(sunxi_phys_data)))
			return -EFAULT;
			
		handle = ion_handle_get_by_id(client, data.handle);		
		if (IS_ERR(handle))
		{
			return PTR_ERR(handle);
		}
			
		ret = ion_phys(client, handle, (ion_phys_addr_t *)&data.phys_addr, (size_t *)&data.size);
		ion_handle_put(handle);
		if(ret)
		{
			return -EINVAL;
		}
		if(copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	default:
               pr_err("%s(%d) err: cmd not support!\n", __func__, __LINE__);
		return -ENOTTY;
	}
	
	return ret;
}

struct ion_client *sunxi_ion_client_create(const char *name)
{
	/*
	 * The assumption is that if there is a NULL device, the ion
	 * driver has not yet probed.
	 */
	if (ion_device == NULL)
		return ERR_PTR(-EPROBE_DEFER);

	if (IS_ERR(ion_device))
		return (struct ion_client *)ion_device;
		
	return ion_client_create(ion_device , name);
}
EXPORT_SYMBOL(sunxi_ion_client_create);


int sunxi_ion_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct ion_platform_heap heaps_desc;
	struct device_node *heap_node = NULL;	

	ion_device = ion_device_create(sunxi_ion_ioctl);
	if(IS_ERR_OR_NULL(ion_device)) 
	{
		return PTR_ERR(ion_device);
	}

	do{
		u32 type = -1;
		struct ion_heap *pheap = NULL;
		
		/*loop all the child node */
		heap_node = of_get_next_child(np , heap_node);
		if(!heap_node)
			break;
		memset( &heaps_desc , 0 , sizeof(heaps_desc) );
		
		/* get the properties "name","type" for common ion heap	*/
		if(of_property_read_u32(heap_node , "type" , &type) )
		{	
			pr_err("You need config the heap node 'type'\n");
			continue;
		}
		heaps_desc.type = type;
		heaps_desc.id = type;

		if(of_property_read_string(heap_node , "name" , &heaps_desc.name) )
		{ 
			pr_err("You need config the heap node 'name'\n");
			continue;
		}

		/*for specail heaps , need extra argument to config */
		if( ION_HEAP_TYPE_CARVEOUT == heaps_desc.type )
		{
			u32 base = 0 , size = 0;
			if( of_property_read_u32( heap_node , "base" , &base) )
				pr_err("You need config the carvout 'base'\n");
			heaps_desc.base = base;
			if( of_property_read_u32( heap_node , "size" , &size) )
				pr_err("You need config the carvout 'size'\n"); 
			heaps_desc.size = size;
		}else if( ION_HEAP_TYPE_DMA == heaps_desc.type )
		{
			heaps_desc.priv = &(pdev->dev);
		}

		/* now we can create a heap & add it to the ion device*/
		pheap = ion_heap_create(&heaps_desc);
		if(IS_ERR_OR_NULL(pheap)) 
		{
			pr_err("ion_heap_create '%s' failured!\n" , heaps_desc.name );
			continue;
		}

		ion_device_add_heap(ion_device , pheap );		
	}while(1);

	return 0;
}

static const struct of_device_id sunxi_ion_dt_ids[] = {
	{ .compatible = "allwinner,sunxi-ion" },
	{ /* sentinel */ }
};

static struct platform_driver sunxi_ion_driver = {
	.driver = {
		.name = "sunxi-ion",
		.of_match_table = sunxi_ion_dt_ids,
	},
	.probe = sunxi_ion_probe,
};
module_platform_driver(sunxi_ion_driver);

