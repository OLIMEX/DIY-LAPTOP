
/* 
 ***************************************************************************************
 * 
 * sunxi_csi.c
 * 
 * Hawkview ISP - sunxi_csi.c module
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
#include <media/sunxi_camera.h>
#include "bsp_csi.h"
#include "sunxi_csi.h"
#include "../vfe_os.h"
#include "../platform_cfg.h"
#define CSI_MODULE_NAME "vfe_csi"

#define IS_FLAG(x,y) (((x)&(y)) == y)

struct csi_dev *csi_gbl[2];

static int sunxi_csi_subdev_s_power(struct v4l2_subdev *sd, int enable)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	if(enable)
		bsp_csi_enable(csi->csi_sel);
	else
		bsp_csi_disable(csi->csi_sel);
	return 0;
}
static int sunxi_csi_subdev_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	if(enable)
		if (csi->capture_mode == V4L2_MODE_IMAGE) 
			bsp_csi_cap_start(csi->csi_sel, csi->bus_info.ch_total_num,CSI_SCAP);
		else
			bsp_csi_cap_start(csi->csi_sel, csi->bus_info.ch_total_num,CSI_VCAP);
	else
		if (csi->capture_mode == V4L2_MODE_IMAGE) 
			bsp_csi_cap_stop(csi->csi_sel, csi->bus_info.ch_total_num,CSI_SCAP);
		else
			bsp_csi_cap_stop(csi->csi_sel, csi->bus_info.ch_total_num,CSI_VCAP);		
	return 0;
}
static int sunxi_csi_subdev_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	csi->capture_mode = param->parm.capture.capturemode;
	return 0;
}

static int sunxi_csi_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		      struct v4l2_subdev_mbus_code_enum *code)
{
	return 0;
}

static int sunxi_csi_subdev_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	return 0;
}

static int sunxi_csi_subdev_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	int i,ret;

	for(i = 0; i < csi->bus_info.ch_total_num; i++)
		csi->bus_info.bus_ch_fmt[i] = (enum bus_pixelcode)fmt->format.code;
    
	for(i = 0; i < csi->bus_info.ch_total_num; i++) {
		csi->frame_info.pix_ch_fmt[i] = pix_fmt_v4l2_to_common(fmt->reserved[0]);
		csi->frame_info.ch_field[i] = field_fmt_v4l2_to_common(fmt->format.field);
		csi->frame_info.ch_size[i].width = fmt->format.width;
		csi->frame_info.ch_size[i].height = fmt->format.height; 
		csi->frame_info.ch_offset[i].hoff = fmt->format.reserved[0];
		csi->frame_info.ch_offset[i].voff = fmt->format.reserved[1];
	}
	csi->frame_info.arrange = csi->arrange;

	ret = bsp_csi_set_fmt(csi->csi_sel, &csi->bus_info,&csi->frame_info);
	if (ret < 0) {
		vfe_err("bsp_csi_set_fmt error at %s!\n",__func__);
		return -1;
	}

	ret = bsp_csi_set_size(csi->csi_sel, &csi->bus_info,&csi->frame_info);
	if (ret < 0) {
		vfe_err("bsp_csi_set_size error at %s!\n",__func__);
		return -1;
	}
	return 0;
}

int sunxi_csi_addr_init(struct v4l2_subdev *sd, u32 val)
{
	return 0;
}
static int sunxi_csi_subdev_cropcap(struct v4l2_subdev *sd,
				       struct v4l2_cropcap *a)
{
	return 0;
}

static int sunxi_csi_s_mbus_config(struct v4l2_subdev *sd,
           const struct v4l2_mbus_config *cfg)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	
	if (cfg->type == V4L2_MBUS_CSI2) {
		csi->bus_info.bus_if = CSI2;
		csi->bus_info.ch_total_num = 0;
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_0)) {
			csi->bus_info.ch_total_num++;
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_1)) {
			csi->bus_info.ch_total_num++;  
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_2)) {
			csi->bus_info.ch_total_num++;
		} 
		if (IS_FLAG(cfg->flags,V4L2_MBUS_CSI2_CHANNEL_3)) {
			csi->bus_info.ch_total_num++;
		}
		vfe_print("csi->bus_info.ch_total_num = %d\n",csi->bus_info.ch_total_num);
	} else if (cfg->type == V4L2_MBUS_PARALLEL) {
		csi->bus_info.bus_if = PARALLEL;
		if(IS_FLAG(cfg->flags,V4L2_MBUS_MASTER)) {
			if(IS_FLAG(cfg->flags,V4L2_MBUS_HSYNC_ACTIVE_HIGH)) {
				csi->bus_info.bus_tmg.href_pol = ACTIVE_HIGH;
			} else {
				csi->bus_info.bus_tmg.href_pol = ACTIVE_LOW;
			}
			if(IS_FLAG(cfg->flags,V4L2_MBUS_VSYNC_ACTIVE_HIGH)) {
				csi->bus_info.bus_tmg.vref_pol = ACTIVE_HIGH;
			} else {
				csi->bus_info.bus_tmg.vref_pol = ACTIVE_LOW;
			}
			if(IS_FLAG(cfg->flags,V4L2_MBUS_PCLK_SAMPLE_RISING)) {
				csi->bus_info.bus_tmg.pclk_sample = RISING;
			} else {
				csi->bus_info.bus_tmg.pclk_sample = FALLING;
			}
			if(IS_FLAG(cfg->flags,V4L2_MBUS_FIELD_EVEN_HIGH)) {
				csi->bus_info.bus_tmg.field_even_pol = ACTIVE_HIGH;
			} else {
				csi->bus_info.bus_tmg.field_even_pol = ACTIVE_LOW;
			}
		} else {
			vfe_err("Do not support MBUS SLAVE!cfg->type = %d\n",cfg->type);
			return -1;
		}
	} else if (cfg->type == V4L2_MBUS_BT656) {
		csi->bus_info.bus_if = BT656;
	}

	return 0;
}

static int sunxi_csi_get_frmsize(struct csi_dev *csi, unsigned int *arg)
{
	*arg = csi->frame_info.frm_byte_size;
	printk("csi->frame_info.frm_byte_size = %d\n",csi->frame_info.frm_byte_size);
	return 0;
}

static long sunxi_csi_subdev_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	int ret = 0;

	switch (cmd) {
	case VIDIOC_SUNXI_CSI_GET_FRM_SIZE:
		mutex_lock(&csi->subdev_lock);
		ret = sunxi_csi_get_frmsize(csi, arg);
		mutex_unlock(&csi->subdev_lock);
		break;
	default:
		return -ENOIOCTLCMD;
	}

	return ret;
}

static const struct v4l2_subdev_core_ops sunxi_csi_core_ops = {
	.s_power = sunxi_csi_subdev_s_power,
	.init = sunxi_csi_addr_init,
	.ioctl = sunxi_csi_subdev_ioctl,
};

static const struct v4l2_subdev_video_ops sunxi_csi_subdev_video_ops = {
	.s_stream = sunxi_csi_subdev_s_stream,
	.cropcap = sunxi_csi_subdev_cropcap,
	.s_mbus_config = sunxi_csi_s_mbus_config,
	.s_parm = sunxi_csi_subdev_s_parm,
};

static const struct v4l2_subdev_pad_ops sunxi_csi_subdev_pad_ops = {
	.enum_mbus_code = sunxi_csi_enum_mbus_code,
	.get_fmt = sunxi_csi_subdev_get_fmt,
	.set_fmt = sunxi_csi_subdev_set_fmt,
};


static struct v4l2_subdev_ops sunxi_csi_subdev_ops = {
	.core = &sunxi_csi_core_ops,
	.video = &sunxi_csi_subdev_video_ops,
	.pad = &sunxi_csi_subdev_pad_ops,
};

static int sunxi_csi_subdev_init(struct csi_dev *csi)
{
	struct v4l2_subdev *sd = &csi->subdev;
	mutex_init(&csi->subdev_lock);
	csi->arrange.row = 1;
	csi->arrange.column = 1;
	csi->bus_info.ch_total_num = 1;	
	v4l2_subdev_init(sd, &sunxi_csi_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(sd->name, sizeof(sd->name), "sunxi_csi.%u", csi->csi_sel);

	v4l2_set_subdevdata(sd, csi);
	return 0;
}

static int csi_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
	struct csi_dev *csi = NULL;
	struct csi_platform_data *pdata = NULL;
	int ret = 0;

	if (np == NULL) {
		vfe_err("CSI failed to get of node\n");
		return -ENODEV;
	}
	csi = kzalloc(sizeof(struct csi_dev), GFP_KERNEL);
	if (!csi) {
		ret = -ENOMEM;
		goto ekzalloc;
	}
	pdata = kzalloc(sizeof(struct csi_platform_data), GFP_KERNEL);
	if (pdata == NULL) {
        ret = -ENOMEM;
        goto freedev;
	}
	pdev->dev.platform_data = pdata;

	pdev->id = of_alias_get_id(np, "csi_res");
	if (pdev->id < 0) {
		vfe_err("CSI failed to get alias id\n");
		ret = -EINVAL;
		goto freepdata;
	}
	pdata->csi_sel = pdev->id;

 	csi->base = of_iomap(np, 0);
	if (!csi->base) {
		ret = -EIO;
		goto freepdata;
	}
	csi->csi_sel = pdata->csi_sel;	
	spin_lock_init(&csi->slock);
	init_waitqueue_head(&csi->wait);
	
	ret = bsp_csi_set_base_addr(csi->csi_sel, (unsigned long)csi->base);
	if(ret < 0)
		goto ehwinit;

	csi_gbl[csi->csi_sel] = csi;
	sunxi_csi_subdev_init(csi);

	platform_set_drvdata(pdev, csi);
	vfe_print("csi probe end csi_sel = %d!\n",pdata->csi_sel);

	return 0;

ehwinit:
	iounmap(csi->base);
freepdata:
    kfree(pdata);
freedev:
	kfree(csi);
ekzalloc:
	vfe_print("csi probe err!\n");
	return ret;
}


static int csi_remove(struct platform_device *pdev)
{
	struct csi_dev *csi = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);
	free_irq(csi->irq, csi);
	mutex_destroy(&csi->subdev_lock);
	if(csi->base)
		iounmap(csi->base);
	kfree(csi);
	kfree(pdev->dev.platform_data);
	return 0;
}

static const struct of_device_id sunxi_csi_match[] = {
	{ .compatible = "allwinner,sunxi-csi", },
	{},
};

MODULE_DEVICE_TABLE(of, sunxi_csi_match);

static struct platform_driver csi_platform_driver = {
	.probe    = csi_probe,
	.remove   = csi_remove,
	.driver = {
		.name   = CSI_MODULE_NAME,
		.owner  = THIS_MODULE,
        .of_match_table = sunxi_csi_match,
	}    
};

void sunxi_csi_dump_regs(struct v4l2_subdev *sd)
{
	struct csi_dev *csi = v4l2_get_subdevdata(sd);
	int i = 0;
	printk("Vfe dump CSI regs :\n");
	for(i = 0; i < 0xb0; i = i + 4)
	{
		if(i % 0x10 == 0)	
			printk("0x%08x:    ", i);
		printk("0x%08x, ", readl(csi->base + i));
		if(i % 0x10 == 0xc)	
			printk("\n");
	}
}

int sunxi_csi_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd)
{
	return v4l2_device_register_subdev(v4l2_dev, sd);
}

void sunxi_csi_unregister_subdev(struct v4l2_subdev *sd)
{
	v4l2_device_unregister_subdev(sd);
	v4l2_set_subdevdata(sd, NULL);
}

int sunxi_csi_get_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = &csi_gbl[sel]->subdev;
	csi_gbl[sel]->csi_sel = sel;
	return (csi_gbl[sel]->use_cnt++);
}
int sunxi_csi_put_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = NULL;
	return (csi_gbl[sel]->use_cnt--);
}

int sunxi_csi_platform_register(void)
{
	int ret;

	ret = platform_driver_register(&csi_platform_driver);
	if (ret) {
		vfe_err("platform driver register failed\n");
		return ret;
	}
	vfe_print("csi_init end\n");
	return 0;
}

void sunxi_csi_platform_unregister(void)
{
	vfe_print("csi_exit start\n");
	platform_driver_unregister(&csi_platform_driver);
	vfe_print("csi_exit end\n");
}

