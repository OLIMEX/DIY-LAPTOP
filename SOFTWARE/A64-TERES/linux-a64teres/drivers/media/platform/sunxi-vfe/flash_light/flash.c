 /* 
  ***************************************************************************************
  * 
  * sunxi_flash.c
  * 
  * Hawkview ISP - sunxi_flash.c module
  * 
  * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
  * 
  * Version 	   Author		  Date			 Description
  * 
  *   3.0		   Yang Feng	 2015/02/27  ISP Tuning Tools Support
  * 
  ****************************************************************************************
  */
	 
#include <linux/kernel.h>
#include <linux/module.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>
#include "../vfe_os.h"
#include "../platform_cfg.h"	 
#include "../vfe.h"

#include "flash.h"

#define FLASH_EN_POL 1
#define FLASH_MODE_POL 1 

struct flash_dev *flash_gbl;
static int flash_power_flag = 0;
 
int io_set_flash_ctrl(struct v4l2_subdev *sd, enum sunxi_flash_ctrl ctrl)
{
	int ret=0;
	unsigned int flash_en, flash_dis, flash_mode, torch_mode;
	struct flash_dev_info *fls_info = &flash_gbl->fl_info;

	if(NULL == fls_info || NULL == sd)
	{
		vfe_err("error flash config!\n");
		return -1;
	}
	flash_en=(fls_info->en_pol!=0)?1:0;	   
	flash_dis=!flash_en;					   
	flash_mode=(fls_info->fl_mode_pol!=0)?1:0;
	torch_mode=!flash_mode;				   

	if(FLASH_RELATING == fls_info->flash_driver_ic)
	{
		switch(ctrl) {
		case SW_CTRL_FLASH_OFF:
			vfe_dbg(0,"FLASH_RELATING SW_CTRL_FLASH_OFF\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output
			ret|=vfe_gpio_write(sd, FLASH_EN, flash_dis);
			ret|=vfe_gpio_write(sd, FLASH_MODE, torch_mode);
			//vfe_gpio_set_status(sd,FLASH_EN,0);//set the gpio to hi-z
			//vfe_gpio_set_status(sd,FLASH_MODE,0);//set the gpio to hi-z
			break;
		case SW_CTRL_FLASH_ON:
			vfe_dbg(0,"FLASH_RELATING SW_CTRL_FLASH_ON\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output
			ret|=vfe_gpio_write(sd, FLASH_MODE, flash_mode);
			ret|=vfe_gpio_write(sd, FLASH_EN, flash_en);
			break;
		case SW_CTRL_TORCH_ON:
			vfe_dbg(0,"FLASH_RELATING SW_CTRL_TORCH_ON\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output
			ret|=vfe_gpio_write(sd, FLASH_MODE, torch_mode);
			ret|=vfe_gpio_write(sd, FLASH_EN, flash_en);
			break;
		default:
			return -EINVAL;
		}
	}
	else if(FLASH_EN_INDEPEND == fls_info->flash_driver_ic)
	{
		switch(ctrl) {
		case SW_CTRL_FLASH_OFF:
			vfe_dbg(0,"FLASH_EN_INDEPEND SW_CTRL_FLASH_OFF\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output
			ret|=vfe_gpio_write(sd, FLASH_EN, 0);
			ret|=vfe_gpio_write(sd, FLASH_MODE, 0);
			//vfe_gpio_set_status(sd,FLASH_EN,0);//set the gpio to hi-z
			//vfe_gpio_set_status(sd,FLASH_MODE,0);//set the gpio to hi-z
			break;
		case SW_CTRL_FLASH_ON:
			vfe_dbg(0,"FLASH_EN_INDEPEND SW_CTRL_FLASH_ON\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output

			ret|=vfe_gpio_write(sd, FLASH_MODE, 1);
			ret|=vfe_gpio_write(sd, FLASH_EN, 0);
			break;
		case SW_CTRL_TORCH_ON:
			vfe_dbg(0,"FLASH_EN_INDEPEND SW_CTRL_TORCH_ON\n");
			vfe_gpio_set_status(sd,FLASH_EN,1);//set the gpio to output
			vfe_gpio_set_status(sd,FLASH_MODE,1);//set the gpio to output

			ret|=vfe_gpio_write(sd, FLASH_MODE, 0);
			ret|=vfe_gpio_write(sd, FLASH_EN, 1);
			break;
		default:
			return -EINVAL;				 
		}
	}
	else
	{
		switch(ctrl) {
		case SW_CTRL_FLASH_OFF:			
			vfe_dbg(0,"FLASH_POWER SW_CTRL_FLASH_OFF\n");
			if (1 == flash_power_flag)
			{
				vfe_set_pmu_channel(sd,FLVDD,OFF);
				flash_power_flag --;
			}
			break;
		case SW_CTRL_FLASH_ON:			
			vfe_dbg(0,"FLASH_POWER SW_CTRL_FLASH_ON\n");			
			if (0 == flash_power_flag)
			{
				vfe_set_pmu_channel(sd,FLVDD,ON);				
				flash_power_flag ++;
			}
			break;
		case SW_CTRL_TORCH_ON:
			vfe_dbg(0,"FLASH_POWER SW_CTRL_TORCH_ON\n");
			if (0 == flash_power_flag)
			{
				vfe_set_pmu_channel(sd,FLVDD,ON);				
				flash_power_flag ++;
			}
			break;
		default:
			return -EINVAL;				 
		}
	}
	if(ret!=0)
	{
		vfe_dbg(0,"flash set ctrl fail, force shut off\n");
		ret|=vfe_gpio_write(sd, FLASH_EN, flash_dis);
		ret|=vfe_gpio_write(sd, FLASH_MODE, torch_mode);
	}
	return ret;
}

int sunxi_flash_check_to_start(struct v4l2_subdev *sd, enum sunxi_flash_ctrl ctrl)
{
	struct vfe_dev *dev=(struct vfe_dev *)dev_get_drvdata(sd->v4l2_dev->dev);
	unsigned int flag, to_flash;

	if (flash_gbl->fl_info.flash_mode ==V4L2_FLASH_LED_MODE_FLASH) {
		to_flash = 1;
	} else if (flash_gbl->fl_info.flash_mode ==V4L2_FLASH_LED_MODE_AUTO) {	
		v4l2_subdev_call(dev->sd, core, ioctl, GET_FLASH_FLAG, &flag);		
		if(flag)
			to_flash = 1;
		else
			to_flash = 0;
	} else {
		to_flash = 0;
	}
	
	if(to_flash)
		io_set_flash_ctrl(sd, ctrl);

	return 0;
}

int sunxi_flash_stop(struct v4l2_subdev *sd)
{
	io_set_flash_ctrl(sd, SW_CTRL_FLASH_OFF);
	return 0;
}

 static int config_flash_mode(struct v4l2_subdev *sd, enum v4l2_flash_led_mode mode, struct flash_dev_info *fls_info)
{
	if(fls_info==NULL)
	{
		vfe_err("camera flash not support!\n");
		return -1;
	}
	if((fls_info->light_src!=0x01)&&(fls_info->light_src!=0x02)&&(fls_info->light_src!=0x10))
	{
		vfe_err("unsupported light source, force LEDx1\n");
		fls_info->light_src=0x01;
	}
	fls_info->flash_mode = mode;
	if(mode == V4L2_FLASH_LED_MODE_TORCH)
	{
		io_set_flash_ctrl(sd, SW_CTRL_TORCH_ON);
	}
	else if(mode == V4L2_FLASH_LED_MODE_NONE)
	{
		io_set_flash_ctrl(sd, SW_CTRL_FLASH_OFF);
	}
	return 0;
}
 
static int sunxi_flash_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	switch (qc->id) {
	case V4L2_CID_FLASH_LED_MODE:
		return v4l2_ctrl_query_fill(qc, 0, 5, 1, 0);
	default:
		break;
	}
	return -EINVAL;
}
 
static int sunxi_flash_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_FLASH_LED_MODE:
		ctrl->value = flash_gbl->fl_info.flash_mode;
		return 0;
	default:
		break;
	}
	return -EINVAL;
}
 
 static int sunxi_flash_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
 {
	int ret;
	struct v4l2_queryctrl qc;
	qc.id = ctrl->id;
	ret = sunxi_flash_queryctrl(sd, &qc);
	if (ret < 0) {
		return ret;
	} 
	if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
		return -ERANGE;
	}
	 
	switch (ctrl->id) {
	case V4L2_CID_FLASH_LED_MODE:		
		return config_flash_mode(sd, ctrl->value, &flash_gbl->fl_info);
	default:
		break;
	}
	return -EINVAL;
 }
 static const struct v4l2_subdev_core_ops sunxi_flash_core_ops = {
	.g_ctrl = sunxi_flash_g_ctrl,
	.s_ctrl = sunxi_flash_s_ctrl,
 };

 static struct v4l2_subdev_ops sunxi_flash_subdev_ops = {
	 .core = &sunxi_flash_core_ops,
 };
 
 int sunxi_flash_info_init(struct v4l2_subdev *sd)
{
	struct vfe_dev *dev=(struct vfe_dev *)dev_get_drvdata(sd->v4l2_dev->dev);
	if(dev->flash_used ==1)
	{
		flash_gbl->fl_info.dev_if=0;
		flash_gbl->fl_info.en_pol=FLASH_EN_POL;
		flash_gbl->fl_info.fl_mode_pol=FLASH_MODE_POL;
		flash_gbl->fl_info.light_src=0x01;
		flash_gbl->fl_info.flash_intensity=400;
		flash_gbl->fl_info.flash_level=0x01;
		flash_gbl->fl_info.torch_intensity=200;
		flash_gbl->fl_info.torch_level=0x01;
		flash_gbl->fl_info.timeout_counter=300*1000;		
		flash_gbl->fl_info.flash_driver_ic = dev->flash_type;
		
		config_flash_mode(sd, V4L2_FLASH_LED_MODE_NONE, &flash_gbl->fl_info);
	}
	return 0;
}
static int sunxi_flash_subdev_init(struct flash_dev *flash)
{
	struct v4l2_subdev *sd = &flash->subdev;
	v4l2_subdev_init(sd, &sunxi_flash_subdev_ops);
	snprintf(sd->name, sizeof(sd->name), "sunxi_flash");
	v4l2_set_subdevdata(sd, flash);
	return 0;
}

static int sunxi_flash_probe(void)
{
 	struct flash_dev *flash = NULL;
	int ret = 0;
	flash = kzalloc(sizeof(struct flash_dev), GFP_KERNEL);
	if (!flash) {
		ret = -ENOMEM;
		vfe_err("sunxi flash kzalloc failed!\n");
		goto ekzalloc;
	}

	flash_gbl = flash;	
	sunxi_flash_subdev_init(flash);
ekzalloc:
	return ret;
}

static int sunxi_flash_remove(struct flash_dev *flash)
{
	kfree(flash);
	return 0;
}

int sunxi_flash_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd)
{
	return v4l2_device_register_subdev(v4l2_dev, sd);
}
 
void sunxi_flash_unregister_subdev(struct v4l2_subdev *sd)
{
	v4l2_device_unregister_subdev(sd);
	v4l2_set_subdevdata(sd, NULL);
}
 
 int sunxi_flash_get_subdev(struct v4l2_subdev **sd, int sel)
 {	
	*sd = &flash_gbl->subdev;
	return 0;
 }
 int sunxi_flash_put_subdev(struct v4l2_subdev **sd, int sel)
 {
	 *sd = NULL;	 
	 return 0;
 }
 
 int sunxi_flash_platform_register(void)
{
	sunxi_flash_probe();
	vfe_print("flash_init end\n");
	return 0;
}

void sunxi_flash_platform_unregister(void)
{
	vfe_print("flash_exit start\n");
	sunxi_flash_remove(flash_gbl);
	vfe_print("flash_exit end\n");
}
