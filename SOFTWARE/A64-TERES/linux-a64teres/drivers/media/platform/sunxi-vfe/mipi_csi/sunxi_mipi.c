
/* 
 ***************************************************************************************
 * 
 * sunxi_mipi.c
 * 
 * Hawkview ISP - sunxi_mipi.c module
 * 
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2015/02/27	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>
#include "bsp_mipi_csi.h"
#include "sunxi_mipi.h"
#include "../vfe_os.h"
#include "../platform_cfg.h"
#define MIPI_MODULE_NAME "vfe_mipi"

#define IS_FLAG(x,y) (((x)&(y)) == y)

struct mipi_dev *mipi_gbl;

static void mipi_release(struct device *dev)
{
	vfe_print("mipi_device_release\n");
};

static enum pkt_fmt get_pkt_fmt(enum bus_pixelcode bus_pix_code)
{
	switch(bus_pix_code) {
		case BUS_FMT_RGB565_16X1:
			return MIPI_RGB565;
		case BUS_FMT_UYVY8_16X1:
			return MIPI_YUV422;
		case BUS_FMT_UYVY10_20X1:
			return MIPI_YUV422_10;
		case BUS_FMT_SBGGR8_8X1:
		case BUS_FMT_SGBRG8_8X1:
		case BUS_FMT_SGRBG8_8X1:      
		case BUS_FMT_SRGGB8_8X1:
			return MIPI_RAW8;
		case BUS_FMT_SBGGR10_10X1:
		case BUS_FMT_SGBRG10_10X1:
		case BUS_FMT_SGRBG10_10X1:      
		case BUS_FMT_SRGGB10_10X1:
			return MIPI_RAW10;
		case BUS_FMT_SBGGR12_12X1:
		case BUS_FMT_SGBRG12_12X1:    
		case BUS_FMT_SGRBG12_12X1:
		case BUS_FMT_SRGGB12_12X1:
			return MIPI_RAW12;
		case BUS_FMT_YY8_UYVY8_12X1:
			return MIPI_YUV420;
		case BUS_FMT_YY10_UYVY10_15X1:
			return MIPI_YUV420_10;
		default:
			return MIPI_RAW8;
	}
}

static int sunxi_mipi_subdev_s_power(struct v4l2_subdev *sd, int enable)
{
	return 0;
}
static int sunxi_mipi_subdev_s_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static int sunxi_mipi_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		      struct v4l2_subdev_mbus_code_enum *code)
{
	return 0;
}

static int sunxi_mipi_subdev_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	return 0;
}

static int sunxi_mipi_subdev_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	struct mipi_dev *mipi = v4l2_get_subdevdata(sd);
	int i;
	
	mipi->mipi_para.bps = fmt->reserved[0];
	mipi->mipi_para.auto_check_bps = 0;	//TODO
	mipi->mipi_para.dphy_freq = DPHY_CLK; //TODO
	
	for(i = 0; i < mipi->mipi_para.total_rx_ch; i++) {//TODO 
		mipi->mipi_fmt.packet_fmt[i] = get_pkt_fmt((enum bus_pixelcode)fmt->format.code);
		mipi->mipi_fmt.field[i] = field_fmt_v4l2_to_common(fmt->format.field);
		mipi->mipi_fmt.vc[i] = i;
	}
	bsp_mipi_csi_dphy_init(mipi->mipi_sel);
	bsp_mipi_csi_set_para(mipi->mipi_sel, &mipi->mipi_para);
	bsp_mipi_csi_set_fmt(mipi->mipi_sel, mipi->mipi_para.total_rx_ch, &mipi->mipi_fmt);
	
	//for dphy clock async
	bsp_mipi_csi_dphy_disable(mipi->mipi_sel);

	return 0;
}

int sunxi_mipi_addr_init(struct v4l2_subdev *sd, u32 val)
{
	return 0;
}
static int sunxi_mipi_subdev_cropcap(struct v4l2_subdev *sd,
				       struct v4l2_cropcap *a)
{
	return 0;
}

static int sunxi_mipi_s_mbus_config(struct v4l2_subdev *sd,
           const struct v4l2_mbus_config *cfg)
{
	struct mipi_dev *mipi = v4l2_get_subdevdata(sd);
	
	if (cfg->type == V4L2_MBUS_CSI2) {
		if(IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_4_LANE))
			mipi->mipi_para.lane_num = 4;
		else if(IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_3_LANE))
			mipi->mipi_para.lane_num = 3;
		else if(IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_2_LANE))
			mipi->mipi_para.lane_num = 2;
		else
			mipi->mipi_para.lane_num = 1;
		
		mipi->mipi_para.total_rx_ch = 0;
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_0)) {
			mipi->mipi_para.total_rx_ch++;
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_1)) {
			mipi->mipi_para.total_rx_ch++;  
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_2)) {
			mipi->mipi_para.total_rx_ch++; 
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_3)) {
			mipi->mipi_para.total_rx_ch++;
		}
	}

	return 0;
}

static const struct v4l2_subdev_core_ops sunxi_mipi_core_ops = {
	.s_power = sunxi_mipi_subdev_s_power,
	.init = sunxi_mipi_addr_init,
};

static const struct v4l2_subdev_video_ops sunxi_mipi_subdev_video_ops = {
	.s_stream = sunxi_mipi_subdev_s_stream,
	.cropcap = sunxi_mipi_subdev_cropcap,
	.s_mbus_config = sunxi_mipi_s_mbus_config,
};

static const struct v4l2_subdev_pad_ops sunxi_mipi_subdev_pad_ops = {
	.enum_mbus_code = sunxi_mipi_enum_mbus_code,
	.get_fmt = sunxi_mipi_subdev_get_fmt,
	.set_fmt = sunxi_mipi_subdev_set_fmt,
};


static struct v4l2_subdev_ops sunxi_mipi_subdev_ops = {
	.core = &sunxi_mipi_core_ops,
	.video = &sunxi_mipi_subdev_video_ops,
	.pad = &sunxi_mipi_subdev_pad_ops,
};

static int sunxi_mipi_subdev_init(struct mipi_dev *mipi)
{
	struct v4l2_subdev *sd = &mipi->subdev;

	v4l2_subdev_init(sd, &sunxi_mipi_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(sd->name, sizeof(sd->name), "sunxi_mipi.%u", mipi->mipi_sel);

	v4l2_set_subdevdata(sd, mipi);
	return 0;
}
static int mipi_probe(struct platform_device *pdev)
{
	struct mipi_dev *mipi = NULL;
	struct resource *res = NULL;
	struct mipi_platform_data *pdata = NULL;
	int ret = 0;
	if(pdev->dev.platform_data == NULL)
	{
		return -ENODEV;
	}
	pdata = pdev->dev.platform_data;
	vfe_print("mipi probe start mipi_sel = %d!\n",pdata->mipi_sel);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (!request_mem_region(res->start, resource_size(res), res->name)) {
		return -ENOMEM;
	}
	mipi = kzalloc(sizeof(struct mipi_dev), GFP_KERNEL);
	if (!mipi) {
		ret = -ENOMEM;
		goto ekzalloc;
	}
	mipi->mipi_sel = pdata->mipi_sel;
	
	spin_lock_init(&mipi->slock);
	init_waitqueue_head(&mipi->wait);
	mipi->base = ioremap(res->start, resource_size(res));
	if (!mipi->base) {
		ret = -EIO;
		goto freedev;
	}

#if defined(CONFIG_ARCH_SUN8IW7P1)

#else
	if(mipi->mipi_sel == 0) {
#if defined (CONFIG_ARCH_SUN8IW6P1)
		bsp_mipi_csi_set_base_addr(mipi->mipi_sel, (unsigned long)mipi->base);
		bsp_mipi_dphy_set_base_addr(mipi->mipi_sel, (unsigned long)mipi->base + 0x1000);
#else
		ret = bsp_mipi_csi_set_base_addr(mipi->mipi_sel, 0);
		if(ret < 0)
			goto ehwinit;
		ret = bsp_mipi_dphy_set_base_addr(mipi->mipi_sel, 0);
		if(ret < 0)
			goto ehwinit;
#endif
	}
#endif
	mipi_gbl = mipi;
	sunxi_mipi_subdev_init(mipi);

	platform_set_drvdata(pdev, mipi);
	vfe_print("mipi probe end mipi_sel = %d!\n",pdata->mipi_sel);
	return 0;

ehwinit:
	iounmap(mipi->base);
freedev:
	kfree(mipi);
ekzalloc:
	vfe_print("mipi probe err!\n");
	return ret;
}


static int mipi_remove(struct platform_device *pdev)
{
	struct mipi_dev *mipi = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);
	if(mipi->base)
		iounmap(mipi->base);
	kfree(mipi);
	return 0;
}

static struct resource mipi0_resource[] = 
{
	[0] = {
		.name	= "mipi",
		.start  = MIPI_CSI0_REGS_BASE,
		.end    = MIPI_CSI0_REGS_BASE + MIPI_CSI_REG_SIZE -1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct mipi_platform_data mipi0_pdata[] = {
	{
		.mipi_sel  = 0,
	},
};

static struct platform_device mipi_device[] = {
	[0] = {
		.name  = MIPI_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(mipi0_resource),
		.resource = mipi0_resource,
		.dev = {
			.platform_data  = mipi0_pdata,
			.release        = mipi_release,
		},
	},
};

static struct platform_driver mipi_platform_driver = {
	.probe    = mipi_probe,
	.remove   = mipi_remove,
	.driver = {
		.name   = MIPI_MODULE_NAME,
		.owner  = THIS_MODULE,
	}
};


int sunxi_mipi_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd)
{
	return v4l2_device_register_subdev(v4l2_dev, sd);
}

void sunxi_mipi_unregister_subdev(struct v4l2_subdev *sd)
{
	v4l2_device_unregister_subdev(sd);
	v4l2_set_subdevdata(sd, NULL);
}

int sunxi_mipi_get_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = &mipi_gbl->subdev;
	return (mipi_gbl->use_cnt++);
}
int sunxi_mipi_put_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = NULL;
	return (mipi_gbl->use_cnt--);
}

int sunxi_mipi_platform_register(void)
{
	int ret,i;
	for(i=0; i<ARRAY_SIZE(mipi_device); i++) 
	{
		ret = platform_device_register(&mipi_device[i]);
		if (ret)
			vfe_err("mipi device %d register failed\n",i);
	}
	ret = platform_driver_register(&mipi_platform_driver);
	if (ret) {
		vfe_err("platform driver register failed\n");
		return ret;
	}
	vfe_print("mipi_init end\n");
	return 0;
}

void sunxi_mipi_platform_unregister(void)
{
	int i;
	vfe_print("mipi_exit start\n");
	for(i=0; i<ARRAY_SIZE(mipi_device); i++)
	{
		platform_device_unregister(&mipi_device[i]);
	}
	platform_driver_unregister(&mipi_platform_driver);
	vfe_print("mipi_exit end\n");
}

