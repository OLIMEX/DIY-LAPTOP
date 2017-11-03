/* linux/drivers/video/sunxi/disp2/disp/dev_debugfs.c
 *
 * Copyright (c) 2014 Allwinnertech Co., Ltd.
 * Author: Tyle <tyle@allwinnertech.com>
 *
 * Display debugfs for sunxi platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "dev_disp_debugfs.h"

static struct dentry *my_dispdbg_root;

struct dispdbg_data{
char command[32];
char name[32];
char start[32];
char param[64];
char info[256];
char tmpbuf[318];
};
static struct dispdbg_data dispdbg_priv;

extern struct disp_layer* disp_get_layer(u32 disp, u32 chn, u32 layer_id);
extern struct disp_layer* disp_get_layer_1(u32 disp, u32 layer_id);
extern struct disp_smbl* disp_get_smbl(u32 disp);
extern struct disp_enhance* disp_get_enhance(u32 disp);
extern struct disp_capture* disp_get_capture(u32 disp);
extern struct disp_device* disp_get_lcd(u32 disp);
extern struct disp_manager* disp_get_layer_manager(u32 disp);
extern unsigned int composer_dump(char* buf);
static void dispdbg_process(void)
{
	int start = simple_strtoul(dispdbg_priv.start,NULL,0);
	if (start != 1)
		return ;

	if (!strncmp(dispdbg_priv.name,"layer",5)) {
		char *p = dispdbg_priv.name + 5;
		int disp,chan,id;
		struct disp_layer *lyr = NULL;

		printk("%s,%s\n", dispdbg_priv.command, dispdbg_priv.name);

		disp = (unsigned int)simple_strtoul(p, &p, 10);
		p++;
		chan = (unsigned int)simple_strtoul(p, &p, 10);
		p++;
		id = (unsigned int)simple_strtoul(p, &p, 10);
		lyr = disp_get_layer(disp, chan, id);
		if (NULL == lyr) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		if (!strncmp(dispdbg_priv.command,"enable",6)) {
			//lyr->enable(lyr);
		} else if (!strncmp(dispdbg_priv.command,"disable",7)) {
			//lyr->disable(lyr);
		} else if (!strncmp(dispdbg_priv.command,"getinfo",7)) {
			lyr->dump(lyr, dispdbg_priv.info);
		} else {
			sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
			return ;
		}
	} else if (!strncmp(dispdbg_priv.name,"lcd",3)) {
		char *p = dispdbg_priv.name + 3;
		int disp;
		struct disp_device *lcd = NULL;

		disp = (unsigned int)simple_strtoul(p, &p, 10);
		lcd = disp_get_lcd(disp);
		if (NULL == lcd) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		if (!strncmp(dispdbg_priv.command,"enable",6)) {
			lcd->enable(lcd);
		} else if (!strncmp(dispdbg_priv.command,"disable",7)) {
			lcd->disable(lcd);
		} else if (!strncmp(dispdbg_priv.command,"setbl",6)) {
			int bl = (unsigned int)simple_strtoul(dispdbg_priv.param, NULL, 10);
			if (lcd->set_bright)
				lcd->set_bright(lcd, bl);
			else
				sprintf(dispdbg_priv.info,"set lcd%d backlight fail", disp);
		} else if (!strncmp(dispdbg_priv.command,"getbl",5)) {
		int bl;
			if (lcd->get_bright) {
				bl = lcd->get_bright(lcd);
				sprintf(dispdbg_priv.info,"%d", bl);
			} else
				sprintf(dispdbg_priv.info,"get lcd%d backlight fail", disp);
		} else {
				sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
				return ;
		}
	}	else if (!strncmp(dispdbg_priv.name,"disp",4)) {
		char *p = dispdbg_priv.name + 4;
		int disp;
		char* next;
		char* tosearch;
		struct disp_manager *mgr = NULL;

		disp = (unsigned int)simple_strtoul(p, &p, 10);
		mgr = disp_get_layer_manager(disp);
		if (NULL == mgr) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		if (!strncmp(dispdbg_priv.command,"getinfo",7)) {
			mgr->dump(mgr, dispdbg_priv.info);
		} else if (!strncmp(dispdbg_priv.command,"switch",6)) {
			u32 type,mode;
			tosearch = dispdbg_priv.param;
			next = strsep(&tosearch, " ");
			type = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			mode = simple_strtoul(next,NULL,0);
			printk("disp %d, type %d, mode%d\n", disp, type, mode);
			bsp_disp_device_switch(disp, type, mode);
		} else if (!strncmp(dispdbg_priv.command,"blank",5)) {
			u32 level;
			struct disp_device *dispdev = mgr->device;

			if (NULL == dispdev) {
				sprintf(dispdbg_priv.info,"get device fail for disp %d!", disp);
				return ;
			}

			level = simple_strtoul(dispdbg_priv.param,NULL,0);
			printk("disp %d, blank%d\n", disp, level);
			if (0 == level)
				dispdev->enable(dispdev);
			else
				dispdev->disable(dispdev);
		} else if (!strncmp(dispdbg_priv.command,"getxres",7)) {
			u32 width, height;
			struct disp_device *dispdev = mgr->device;

			if (NULL == dispdev) {
				sprintf(dispdbg_priv.info,"get device fail for disp %d!", disp);
				return ;
			}
			dispdev->get_resolution(dispdev, &width, &height);

			sprintf(dispdbg_priv.info,"%d", width);
		} else if (!strncmp(dispdbg_priv.command,"getyres",7)) {
			u32 width, height;
			struct disp_device *dispdev = mgr->device;

			if (NULL == dispdev) {
				sprintf(dispdbg_priv.info,"get device fail for disp %d!", disp);
				return ;
			}
			dispdev->get_resolution(dispdev, &width, &height);

			sprintf(dispdbg_priv.info,"%d", height);
		}  else if (!strncmp(dispdbg_priv.command,"getfps",6)) {
			u32 fps = bsp_disp_get_fps(disp);
			u32 count = 0;

			count = sprintf(dispdbg_priv.info,"device:%d.%d fps\n", fps/10, fps%10);
			//composer_dump(dispdbg_priv.info+count);
		}
#if defined(SUPPORT_TV)
		else if (!strncmp(dispdbg_priv.command,"suspend",7)) {
			if (mgr->device) {
				if ((DISP_OUTPUT_TYPE_TV == mgr->device->type)) {
				disp_tv_suspend(mgr->device);
				}
			}

		} else if (!strncmp(dispdbg_priv.command,"resume",6)) {
			if (mgr->device) {
				if ((DISP_OUTPUT_TYPE_TV == mgr->device->type)) {
					disp_tv_resume(mgr->device);
				}
			}

		}
#endif
		else if (!strncmp(dispdbg_priv.command,"vsync_enable",12)) {
			u32 enable;

			enable = simple_strtoul(dispdbg_priv.param,NULL,10);
			bsp_disp_vsync_event_enable(disp, (1==enable)? true:false);
		}
		else {
			sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
			return ;
		}
	}
	else if (!strncmp(dispdbg_priv.name,"enhance",7)) {
		char *p = dispdbg_priv.name + 7;
		int disp;
		char* next;
		char* tosearch;
		struct disp_manager *mgr = NULL;
		struct disp_enhance *enhance = NULL;
		disp_enhance_para para;

		memset(&para, 0, sizeof(disp_enhance_para));
		disp = (unsigned int)simple_strtoul(p, &p, 10);
		mgr = disp_get_layer_manager(disp);
		if (NULL == mgr) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		enhance = mgr->enhance;
		if (NULL == enhance) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		if (!strncmp(dispdbg_priv.command,"setinfo",7)) {
			/* en */
			tosearch = dispdbg_priv.param;
			next = strsep(&tosearch, " ");
			para.enable = simple_strtoul(next,NULL,0);

			/* mode */
			next = strsep(&tosearch, " ");
			para.mode = simple_strtoul(next,NULL,0);

			/* bright/contrast/saturation/hue */
			next = strsep(&tosearch, " ");
			para.bright = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.contrast = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.saturation = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.hue = simple_strtoul(next,NULL,0);

			/* sharp */
			next = strsep(&tosearch, " ");
			para.sharp = simple_strtoul(next,NULL,0);

			/* auto color */
			next = strsep(&tosearch, " ");
			para.auto_contrast = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.auto_color = simple_strtoul(next,NULL,0);

			/* fancycolor */
			next = strsep(&tosearch, " ");
			para.fancycolor_red = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.fancycolor_green = simple_strtoul(next,NULL,0);
			next = strsep(&tosearch, " ");
			para.fancycolor_blue = simple_strtoul(next,NULL,0);

			/* window */
			next = strsep(&tosearch, " ");
			if (!strncmp(next,"win",3)) {
				next = strsep(&tosearch, " ");
				para.window.x = simple_strtoul(next,NULL,0);
				next = strsep(&tosearch, " ");
				para.window.y = simple_strtoul(next,NULL,0);
				next = strsep(&tosearch, " ");
				para.window.width = simple_strtoul(next,NULL,0);
				next = strsep(&tosearch, " ");
				para.window.height = simple_strtoul(next,NULL,0);
			}
			printk("enhance %d, en(%d), mode(%d), bcsh(%d,%d,%d,%d), sharp(%d), autocolor(%d,%d), fancycolor(%d,%d,%d)\n",
				disp, para.enable, para.mode, para.bright, para.contrast, para.saturation,
				para.hue, para.sharp, para.auto_contrast, para.auto_color,
				para.fancycolor_red, para.fancycolor_green, para.fancycolor_blue);
			enhance->set_para(enhance, &para);
		}  else if (!strncmp(dispdbg_priv.command,"getinfo",7)) {
			if (enhance->dump)
				enhance->dump(enhance, dispdbg_priv.info);
		} else {
			sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
			return ;
		}
	}
	else if (!strncmp(dispdbg_priv.name,"smbl",4)) {
		char *p = dispdbg_priv.name + 4;
		int disp;
		struct disp_manager *mgr = NULL;
		struct disp_smbl *smbl = NULL;

		disp = (unsigned int)simple_strtoul(p, &p, 10);
		mgr = disp_get_layer_manager(disp);
		if (NULL == mgr) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		smbl = mgr->smbl;
		if (NULL == smbl) {
			sprintf(dispdbg_priv.info,"get %s fail!", dispdbg_priv.name);
			return ;
		}
		if (!strncmp(dispdbg_priv.command,"setinfo",7)) {

		} else if (!strncmp(dispdbg_priv.command,"getinfo",7)) {
			if (smbl->dump)
				smbl->dump(smbl, dispdbg_priv.info);
		} else {
			sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
			return ;
		}
	} else if (!strncmp(dispdbg_priv.name,"hdmi",4)) {
		char *p = dispdbg_priv.name + 4;
		int disp;
		unsigned int mode;

		disp = (unsigned int)simple_strtoul(p, &p, 10);
		if (!strncmp(dispdbg_priv.command,"is_support",10)) {
			int is_support = 0;
			mode = (unsigned int)simple_strtoul(dispdbg_priv.param, NULL, 10);
			is_support = bsp_disp_hdmi_check_support_mode(disp, (enum disp_output_type)mode);
			sprintf(dispdbg_priv.info,"%d", is_support);
		} else {
			sprintf(dispdbg_priv.info,"not support command for %s!", dispdbg_priv.name);
			return ;
		}
	}
}

//##########command###############
static int dispdbg_command_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dispdbg_command_release(struct inode * inode, struct file * file)
{
	return 0;
}
static ssize_t dispdbg_command_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len	= strlen(dispdbg_priv.command);

	strcpy(dispdbg_priv.tmpbuf,dispdbg_priv.command);
	dispdbg_priv.tmpbuf[len]=0x0A;
	dispdbg_priv.tmpbuf[len+1]=0x0;
	len	= strlen(dispdbg_priv.tmpbuf);
	if (len) {
		if (*ppos >=len)
			return 0;
		if (count >=len)
			count = len;
		if (count > (len - *ppos))
			count = (len - *ppos);

		if (copy_to_user((void __user *)buf,(const void *)dispdbg_priv.tmpbuf,(unsigned long)len)) {
			pr_warn("copy_to_user fail\n");
			return 0;
		}
		*ppos += count;
	}
	else
		count = 0;
	return count;
}

static ssize_t dispdbg_command_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if ( count >= sizeof(dispdbg_priv.command) )
		return 0;
	if (copy_from_user(dispdbg_priv.command, buf, count)) {
		pr_warn("copy_from_user fail\n");
		return 0;
	}

	if (dispdbg_priv.command[count-1]==0x0A)
		dispdbg_priv.command[count-1]=0;
	else
		dispdbg_priv.command[count]=0;
	return count;
}

static int dispdbg_name_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dispdbg_name_release(struct inode * inode, struct file * file)
{
	return 0;
}
static ssize_t dispdbg_name_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len	= strlen(dispdbg_priv.name);

	strcpy(dispdbg_priv.tmpbuf,dispdbg_priv.name);
	dispdbg_priv.tmpbuf[len]=0x0A;
	dispdbg_priv.tmpbuf[len+1]=0x0;
	len	= strlen(dispdbg_priv.tmpbuf);
	if (len)	{
		if (*ppos >=len)
			return 0;
		if (count >=len)
			count = len;
		if (count > (len - *ppos))
			count = (len - *ppos);

		if (copy_to_user((void __user *)buf,(const void *)dispdbg_priv.tmpbuf,(unsigned long)len)) {
			pr_warn("copy_to_user fail\n");
			return 0;
		}

		*ppos += count;
	}
	else
		count = 0;
	return count;
}

static ssize_t dispdbg_name_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if ( count >= sizeof(dispdbg_priv.name) )
		return 0;
	if (copy_from_user(dispdbg_priv.name, buf, count)) {
		pr_warn("copy_from_user fail\n");
		return 0;
	}

	if (dispdbg_priv.name[count-1]==0x0A)
		dispdbg_priv.name[count-1]=0;
	else
		dispdbg_priv.name[count]=0;
	return count;
}

static int dispdbg_param_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dispdbg_param_release(struct inode * inode, struct file * file)
{
	return 0;
}
static ssize_t dispdbg_param_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len	= strlen(dispdbg_priv.param);

	strcpy(dispdbg_priv.tmpbuf,dispdbg_priv.param);
	dispdbg_priv.tmpbuf[len]=0x0A;
	dispdbg_priv.tmpbuf[len+1]=0x0;
	len	= strlen(dispdbg_priv.tmpbuf);
	if (len) {
		if (*ppos >=len)
			return 0;
		if (count >=len)
			count = len;
		if (count > (len - *ppos))
			count = (len - *ppos);
		if (copy_to_user((void __user *)buf,(const void *)dispdbg_priv.tmpbuf,(unsigned long)len)) {
			pr_warn("copy_to_user fail\n");
			return 0;
		}
		*ppos += count;
	}
	else
		count = 0;
	return count;
}
static ssize_t dispdbg_param_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if ( count >= sizeof(dispdbg_priv.param) )
		return 0;
	if (copy_from_user(dispdbg_priv.param, buf, count)) {
		pr_warn("copy_from_user fail\n");
		return 0;
	}

	if (dispdbg_priv.param[count-1]==0x0A)
		dispdbg_priv.param[count-1]=0;
	else
		dispdbg_priv.param[count]=0;
	return count;
}

static int dispdbg_start_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dispdbg_start_release(struct inode * inode, struct file * file)
{
	return 0;
}
static ssize_t dispdbg_start_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len	= strlen(dispdbg_priv.start);

	strcpy(dispdbg_priv.tmpbuf,dispdbg_priv.start);
	dispdbg_priv.tmpbuf[len]=0x0A;
	dispdbg_priv.tmpbuf[len+1]=0x0;
	len	= strlen(dispdbg_priv.tmpbuf);
	if (len)	{
		if (*ppos >=len)
			return 0;
		if (count >=len)
			count = len;
		if (count > (len - *ppos))
			count = (len - *ppos);
		if (copy_to_user((void __user *)buf,(const void *)dispdbg_priv.tmpbuf,(unsigned long)len)) {
			pr_warn("copy_to_user fail\n");
			return 0;
		}
		*ppos += count;
	}
	else
		count = 0;
	return count;
}
static ssize_t dispdbg_start_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if ( count >= sizeof(dispdbg_priv.start) )
		return 0;
	if (copy_from_user(dispdbg_priv.start, buf, count)) {
		pr_warn("copy_from_user fail\n");
		return 0;
	}

	if (dispdbg_priv.start[count-1]==0x0A)
		dispdbg_priv.start[count-1]=0;
	else
		dispdbg_priv.start[count]=0;
	dispdbg_process();
	return count;
}

static int dispdbg_info_open(struct inode * inode, struct file * file)
{
	return 0;
}
static int dispdbg_info_release(struct inode * inode, struct file * file)
{
	return 0;
}

static ssize_t dispdbg_info_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len	= strlen(dispdbg_priv.info);

	strcpy(dispdbg_priv.tmpbuf,dispdbg_priv.info);
	dispdbg_priv.tmpbuf[len]=0x0A;
	dispdbg_priv.tmpbuf[len+1]=0x0;
	len	= strlen(dispdbg_priv.tmpbuf);

	if (len) {
		if (*ppos >=len)
			return 0;
		if (count >=len)
			count = len;
		if (count > (len - *ppos))
			count = (len - *ppos);
		if (copy_to_user((void __user *)buf,(const void *)dispdbg_priv.tmpbuf,(unsigned long)len)) {
			pr_warn("copy_to_user fail\n");
			return 0;
		}
		*ppos += count;
	}
	else
		count = 0;
	return count;
}
static ssize_t dispdbg_info_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if ( count >= sizeof(dispdbg_priv.info) )
		return 0;
	if (copy_from_user(dispdbg_priv.info, buf, count)) {
		pr_warn("copy_from_user fail\n");
		return 0;
	}

	if (dispdbg_priv.info[count-1]==0x0A)
		dispdbg_priv.info[count-1]=0;
	else
		dispdbg_priv.info[count]=0;
	return count;
}
static const struct file_operations command_ops = {
	.write        = dispdbg_command_write,
	.read        = dispdbg_command_read,
	.open        = dispdbg_command_open,
	.release    = dispdbg_command_release,
};
static const struct file_operations name_ops = {
	.write        = dispdbg_name_write,
	.read        = dispdbg_name_read,
	.open        = dispdbg_name_open,
	.release    = dispdbg_name_release,
};

static const struct file_operations start_ops = {
	.write        = dispdbg_start_write,
	.read        = dispdbg_start_read,
	.open        = dispdbg_start_open,
	.release    = dispdbg_start_release,
};
static const struct file_operations param_ops = {
	.write        = dispdbg_param_write,
	.read        = dispdbg_param_read,
	.open        = dispdbg_param_open,
	.release    = dispdbg_param_release,
};
static const struct file_operations info_ops = {
	.write        = dispdbg_info_write,
	.read        = dispdbg_info_read,
	.open        = dispdbg_info_open,
	.release    = dispdbg_info_release,
};

int dispdbg_init(void)
{
	my_dispdbg_root = debugfs_create_dir("dispdbg", NULL);
	if (!debugfs_create_file("command", 0644, my_dispdbg_root, NULL,&command_ops))
		goto Fail;
	if (!debugfs_create_file("name", 0644, my_dispdbg_root, NULL,&name_ops))
		goto Fail;
	if (!debugfs_create_file("start", 0644, my_dispdbg_root, NULL,&start_ops))
		goto Fail;
	if (!debugfs_create_file("param", 0644, my_dispdbg_root, NULL,&param_ops))
		goto Fail;
	if (!debugfs_create_file("info", 0644, my_dispdbg_root, NULL,&info_ops))
		goto Fail;
	return 0;

Fail:
	debugfs_remove_recursive(my_dispdbg_root);
	my_dispdbg_root = NULL;
	return -ENOENT;
}

int dispdbg_exit(void)
{
	if (NULL != my_dispdbg_root) {
		debugfs_remove_recursive(my_dispdbg_root);
		my_dispdbg_root = NULL;
	}
	return 0;
}
