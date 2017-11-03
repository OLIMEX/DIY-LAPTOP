/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2015 Allwinner Technology Co., Ltd.
 *
 * Author: Xiangyun Yu <yuxyun@allwinnertech.com>
 */

#include <linux/aw/platform.h>

extern unsigned long totalram_pages;

struct __fb_addr_para
{
	unsigned int fb_paddr;
	unsigned int fb_size;
};

extern void sunxi_get_fb_addr_para(struct __fb_addr_para *fb_addr_para);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)) && defined (CONFIG_SUNXI_THERMAL)
extern int ths_read_data(int value);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)) && defined (CONFIG_SUNXI_THERMAL) */

#ifdef CONFIG_SUN50IW1P1_THERMAL
extern int sunxi_get_sensor_temp(u32 sensor_num, long *temperature);
#endif /* CONFIG_SUN50IW1P1_THERMAL */

#ifdef CONFIG_SUNXI_GPU_COOLING
extern int gpu_thermal_cool_unregister(void);
#endif /* CONFIG_SUNXI_GPU_COOLING */

static struct mali_gpu_device_data mali_gpu_data;

#ifndef CONFIG_MALI_DT
static struct resource mali_gpu_resources[]=
{
    MALI_GPU_RESOURCES_MALI400_MP2_PMU(GPU_PBASE, IRQ_GPU_GP, IRQ_GPU_GPMMU, IRQ_GPU_PP0, IRQ_GPU_PPMMU0, IRQ_GPU_PP1, IRQ_GPU_PPMMU1)
};

static struct platform_device mali_gpu_device =
{
    .name = MALI_GPU_NAME_UTGARD,
    .id = 0,
};
#endif

/*
***************************************************************
 @Function   :get_temperature
 @Description:Get the temperature of gpu
***************************************************************
*/
static long get_temperature(void)
{
	long temperature = 0;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)) && defined (CONFIG_SUNXI_THERMAL)
		temperature = ths_read_data(private_data.sensor_num);
	#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)) && defined (CONFIG_SUNXI_THERMAL) */
	#ifdef CONFIG_SUN50IW1P1_THERMAL
		if(sunxi_get_sensor_temp(private_data.sensor_num, &temperature))
		{
			MALI_PRINT_ERROR(("Failed to get the temperature information from sensor %d!\n", private_data.sensor_num));
			return -1;
		}
	#endif /* CONFIG_SUN50IW1P1_THERMAL */
	return temperature;
}

/*
***************************************************************
 @Function   :get_gpu_clk
 @Description:Get gpu related clocks
***************************************************************
*/
static bool get_gpu_clk(void)
{
	int i;
	for(i = 0; i < sizeof(clk_data)/sizeof(clk_data[0]); i++)
	{
#ifdef CONFIG_MALI_DT
		clk_data[i].clk_handle = of_clk_get(private_data.np_gpu, i);
#else
		clk_data[i].clk_handle = clk_get(NULL, clk_data[i].clk_id);
#endif /* CONFIG_MALI_DT */
		if(!clk_data[i].clk_handle || IS_ERR(clk_data[i].clk_handle))
		{
			MALI_PRINT_ERROR(("Failed to get gpu %s clock!\n", clk_data[i].clk_name));
			return 0;
		}
	}

	return 1;
}

/*
***************************************************************
 @Function   :get_current_freq
 @Description:Get current frequency of gpu
***************************************************************
*/
static int get_current_freq(void)
{
	return clk_get_rate(clk_data[0].clk_handle)/(1000*1000);
}

/*
***************************************************************
 @Function   :set_clk_freq
 @Description:Set clock's frequency
***************************************************************
*/
static bool set_clk_freq(int freq /* MHz */)
{
	int i;

	for(i = 0; i < sizeof(clk_data)/sizeof(clk_data[0]); i++)
	{
		if(clk_set_rate(clk_data[i].clk_handle, freq*1000*1000))
		{
			MALI_PRINT_ERROR(("Failed to set the frequency of gpu %s clock: Current frequency is %ld MHz, the frequency to be is %d MHz\n", clk_data[i].clk_name, clk_get_rate(clk_data[i].clk_handle)/(1000*1000), freq));
			return 0;
		}
	}

	MALI_PRINT(("Set gpu frequency to %d MHz\n", freq));

	return 1;
}

/*
***************************************************************
 @Function   :set_gpu_freq
 @Description:Set the frequency of gpu
***************************************************************
*/
static void set_gpu_freq(int freq /* MHz */)
{
	if (&private_data.lock)
	{
		mutex_lock(&private_data.lock);
		mali_dev_pause();
		(void)set_clk_freq(freq);
		mali_dev_resume();
		mutex_unlock(&private_data.lock);
	}
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0))
/*
***************************************************************
 @Function   :gpu_thermal_cool
 @Description:Called by thermal cooling system to change gpu
			  frequency according to gpu temperature
***************************************************************
*/
int gpu_thermal_cool(int freq /* MHz */)
{
	if(private_data.tempctrl_data.temp_ctrl_status && freq > 0)
	{
		if(get_current_freq() != freq)
		{
			set_gpu_freq(freq);
		}
		freq_data.max_freq = freq;
	}
	else
	{
		if(get_current_freq() != freq_data.normal_freq)
                {
                        set_gpu_freq(freq_data.normal_freq);
                }
                freq_data.max_freq = freq_data.extreme_freq;
	}

	return 0;
}
EXPORT_SYMBOL(gpu_thermal_cool);
#endif

/*
***************************************************************
 @Function   :set_voltage
 @Description:Set the voltage of gpu
***************************************************************
*/
static void set_voltage(int vol /* mV */)
{
	if (!IS_ERR_OR_NULL(private_data.regulator))
	{
		if(regulator_set_voltage(private_data.regulator, vol*1000, vol*1000) != 0)
		{
			MALI_PRINT_ERROR(("Failed to set gpu voltage!\n"));
		}
	}
}

/*
***************************************************************
 @Function   :enable_gpu_clk
 @Description:Enable gpu related clock gatings
***************************************************************
*/
bool enable_gpu_clk(void)
{
	if(!private_data.clk_status)
	{
		int i;
		for(i=0; i<sizeof(clk_data)/sizeof(clk_data[0]); i++)
		{
			if(clk_prepare_enable(clk_data[i].clk_handle))
			{
				MALI_PRINT_ERROR(("Failed to enable %s clock!\n", clk_data[i].clk_name));
				return 0;
			}
		}
		private_data.clk_status = 1;
	}

	return 1;
}

/*
***************************************************************
 @Function   :disable_gpu_clk
 @Description:Disable gpu related clock gatings
***************************************************************
*/
void disable_gpu_clk(void)
{
	if(private_data.clk_status)
	{
		int i;
		for(i=sizeof(clk_data)/sizeof(clk_data[0])-1; i>=0; i--)
		{
			clk_disable_unprepare(clk_data[i].clk_handle);
		}

		private_data.clk_status = 0;
	}
}

#ifdef CONFIG_MALI_DT
/*
***************************************************************
 @Function   :mali_platform_device_deinit
 @Description:Remove the resource gpu used, called when mali
			  driver is removed
***************************************************************
*/
int mali_platform_device_deinit(struct platform_device *device)
{
	disable_gpu_clk();
#ifdef CONFIG_SUNXI_GPU_COOLING
	gpu_thermal_cool_unregister();
#endif /* CONFIG_SUNXI_GPU_COOLING */
	return 0;
}
#endif /* CONFIG_MALI_DT */

/*
***************************************************************
 @Function   :dvfs_manual_show
 @Description:Show the gpu frequency for manual
***************************************************************
*/
static ssize_t dvfs_manual_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d MHz\n", get_current_freq());
}

/*
***************************************************************
 @Function   :dvfs_manual_store
 @Description:Change gpu frequency for manual
***************************************************************
*/
static ssize_t dvfs_manual_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int err;
	unsigned long freq;

	err = strict_strtoul(buf, 10, &freq);
	if (err)
	{
		MALI_PRINT(("Invalid parameter!\n"));
		goto err_out;
	}

	set_gpu_freq(freq);

err_out:
	return count;
}

/*
***************************************************************
 @Function   :dvfs_android_show
 @Description:Show the gpu frequency for android
***************************************************************
*/
static ssize_t dvfs_android_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return dvfs_manual_show(dev, attr, buf);
}

/*
***************************************************************
 @Function   :dvfs_android_store
 @Description:Change gpu frequency for android
***************************************************************
*/
static ssize_t dvfs_android_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if(private_data.scene_ctrl_status)
	{
		int err;
		unsigned long cmd;

		err = strict_strtoul(buf, 10, &cmd);
		if (err)
		{
			MALI_PRINT(("Invalid parameter!\n"));
			goto out;
		}

		if(cmd == 0)
		{
			if(private_data.tempctrl_data.temp_ctrl_status && freq_data.normal_freq >= freq_data.max_freq)
			{
				goto out;
			}
			else if(get_current_freq() != freq_data.normal_freq)
			{
				/* Recover to normal frequency */
				set_gpu_freq(freq_data.normal_freq);
			}
		}
		else if(cmd == 1)
		{
			if(private_data.tempctrl_data.temp_ctrl_status && freq_data.extreme_freq >= freq_data.max_freq)
			{
				goto out;
			}
			else if(get_current_freq() != freq_data.extreme_freq)
			{
				/* Run in extreme mode */
				set_gpu_freq(freq_data.extreme_freq);
			}
		}
	}

out:
	return count;
}

/*
***************************************************************
 @Function   :status_tempctrl_show
 @Description:Show the temperature control status
***************************************************************
*/
static ssize_t status_tempctrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u32 bufercnt = 0, ret = 0;
#ifdef CONFIG_CPU_BUDGET_THERMAL
	u32 i = 0;
#endif /* CONFIG_CPU_BUDGET_THERMAL */

    ret = sprintf(buf, "sensor: %d, status: %d, temperature: %ld\n", private_data.sensor_num, private_data.tempctrl_data.temp_ctrl_status, get_temperature());

#ifdef CONFIG_CPU_BUDGET_THERMAL
	bufercnt = ret;
    ret = sprintf(buf + bufercnt, "num temperature frequency\n");

    while(i < private_data.tempctrl_data.count)
    {
		bufercnt += ret;
        ret = sprintf(buf+bufercnt, " %d     %3d         %d\n", i, tf_table[i].temp, tf_table[i].freq);
        i++;
    }
#endif /* CONFIG_CPU_BUDGET_THERMAL */

	return bufercnt+ret;
}

/*
***************************************************************
 @Function   :status_tempctrl_store
 @Description:Change the temperature control status
***************************************************************
*/
static ssize_t status_tempctrl_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int err;
	unsigned long status;
	err = strict_strtoul(buf, 10, &status);
	if (err)
	{
		MALI_PRINT(("Invalid parameter!\n"));
		goto out;
	}

	if(status <= 1)
	{
		private_data.tempctrl_data.temp_ctrl_status = status;
	}
	else
	{
		MALI_PRINT(("The parameter is too large!\n"));
	}

out:
	return count;
}

/*
***************************************************************
 @Function   :status_scenectrl_show
 @Description:Show the temperature control status
***************************************************************
*/
static ssize_t status_scenectrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", private_data.scene_ctrl_status);
}

/*
***************************************************************
 @Function   :status_tempctrl_store
 @Description:Change the temperature control status
***************************************************************
*/
static ssize_t status_scenectrl_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int err;
	unsigned long status;
	err = strict_strtoul(buf, 10, &status);
	if (err)
	{
		MALI_PRINT(("Invalid parameter!\n"));
		goto out;
	}

	if(status <= 1)
	{
		private_data.scene_ctrl_status = status;
	}
	else
	{
		MALI_PRINT(("The parameter is too large!\n"));
	}

out:
	return count;
}

/*
***************************************************************
 @Function   :change_voltage_show
 @Description:Show the current voltage of gpu
***************************************************************
*/
static ssize_t change_voltage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	if (!IS_ERR_OR_NULL(private_data.regulator))
	{
		count = sprintf(buf, "%d mV\n", regulator_get_voltage(private_data.regulator)/1000);
	}
	return count;
}

/*
***************************************************************
 @Function   :change_voltage_store
 @Description:Change the voltage of gpu
***************************************************************
*/
static ssize_t change_voltage_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int err;
	unsigned long vol;
	err = strict_strtoul(buf, 10, &vol);
	if (err)
	{
		MALI_PRINT_ERROR(("Invalid parameter!\n"));
		goto out;
	}

	if(vol <= 1300)
	{
		if (&private_data.lock)
		{
			mutex_lock(&private_data.lock);
			mali_dev_pause();
			set_voltage(vol);
			mali_dev_resume();
			mutex_unlock(&private_data.lock);
		}
	}
	else
	{
		MALI_PRINT_ERROR(("The parameter is too large!\n"));
	}

out:
	return count;
}

static DEVICE_ATTR(manual, S_IRUGO|S_IWUGO, dvfs_manual_show, dvfs_manual_store);
static DEVICE_ATTR(android, S_IRUGO|S_IWUGO, dvfs_android_show, dvfs_android_store);
static DEVICE_ATTR(tempctrl, S_IRUGO|S_IWUGO, status_tempctrl_show, status_tempctrl_store);
static DEVICE_ATTR(scenectrl, S_IRUGO|S_IWUGO, status_scenectrl_show, status_scenectrl_store);
static DEVICE_ATTR(voltage, S_IRUGO|S_IWUGO, change_voltage_show, change_voltage_store);

static struct attribute *gpu_attributes[] =
{
    &dev_attr_manual.attr,
	&dev_attr_android.attr,
	&dev_attr_tempctrl.attr,
	&dev_attr_scenectrl.attr,
	&dev_attr_voltage.attr,

    NULL,
};

struct attribute_group gpu_attribute_group = 
{
	.name = "dvfs",
	.attrs = gpu_attributes,
};

#ifdef CONFIG_CPU_BUDGET_THERMAL
static int gpu_throttle_notifier_call(struct notifier_block *nfb, unsigned long mode, void *cmd)
{
    int retval = NOTIFY_DONE;
	int i = 0;
	int cur_freq = get_current_freq();

	if(private_data.tempctrl_data.temp_ctrl_status)
	{
		long temperature = get_temperature();
		if(temperature > tf_table[0].temp)
		{
			{
				for(i = private_data.tempctrl_data.count - 1; i >= 0; i--)
				{
					if(temperature >= tf_table[i].temp)
					{
						if(cur_freq != tf_table[i].freq)
						{
							set_gpu_freq(tf_table[i].freq);
						}
						freq_data.max_freq = tf_table[i].freq;
						break;
					}
				}
			}
		}
	}

	return retval;
}

static struct notifier_block gpu_throttle_notifier = {
.notifier_call = gpu_throttle_notifier_call,
};
#endif /* CONFIG_CPU_BUDGET_THERMAL */

/*
***************************************************************
 @Function   :get_para_from_fex
 @Description:Get a parameter from sys_config.fex
***************************************************************
*/
static int get_para_from_fex(char *main_key, char *second_key, int max_value)
{
	u32 value;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	script_item_u val;
    script_item_value_type_e type;
	type = script_get_item(main_key, second_key, &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
		MALI_PRINT(("%s: %s in sys_config.fex is invalid!\n", main_key, second_key));
		return -1;
	}
	value = val.val;
#else
	if(of_property_read_u32(private_data.np_gpu, second_key, &value) < 0)
	{
		return -1;
	}
#endif
	if(max_value)
	{
		if(value <= max_value)
		{
			return value;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return value;
	}
}

/*
***************************************************************
 @Function   :parse_sysconfig_fex
 @Description:Parse sys_config.fex
***************************************************************
*/
static void parse_sysconfig_fex(void)
{
	int value;
#ifdef CONFIG_CPU_BUDGET_THERMAL
	int i;
	char tfx_name[10] = {0};
#endif /* CONFIG_CPU_BUDGET_THERMAL */

	value = get_para_from_fex("gpu_mali400_0", "normal_freq", 0);
	if(value > 0)
	{
		freq_data.normal_freq = value;
	}

	value = get_para_from_fex("gpu_mali400_0", "scene_ctrl_status", 1);
	if(value != -1)
	{
		private_data.scene_ctrl_status = value;
	}

	value = get_para_from_fex("gpu_mali400_0", "temp_ctrl_status", 1);
	if(value != -1)
	{
		private_data.tempctrl_data.temp_ctrl_status = value;
	}

#ifdef CONFIG_CPU_BUDGET_THERMAL
	value = get_para_from_fex("gpu_mali400_0", "tft_count", sizeof(tf_table)/sizeof(tf_table[0]));
	if(value > 0)
	{
		private_data.tempctrl_data.count = value;
		for(i = 0; i < value; i++)
		{
			sprintf(tfx_name, "tf%d_temp",i);
			value = get_para_from_fex("gpu_mali400_0", tfx_name, 0);
			if(value != -1)
			{
				tf_table[i].temp = value;
			}

			sprintf(tfx_name, "tf%d_freq",i);
			value = get_para_from_fex("gpu_mali400_0", tfx_name, 0);
			if(value != -1)
			{
				tf_table[i].freq = value;
			}
		}
	}
#endif /* CONFIG_CPU_BUDGET_THERMAL */
}

/*
***************************************************************
 @Function   :mali_platform_init
 @Description:Init the clocks of gpu
***************************************************************
*/
static int mali_platform_init(struct platform_device *device)
{
	bool err = 0;

#ifdef CONFIG_CPU_BUDGET_THERMAL
	private_data.tempctrl_data.count = sizeof(tf_table)/sizeof(tf_table[0]);
#endif /* CONFIG_CPU_BUDGET_THERMAL */

#ifdef CONFIG_MALI_DT
	private_data.np_gpu = of_find_compatible_node(NULL, NULL, "arm,mali-400");
#endif /* CONFIG_MALI_DT */

	parse_sysconfig_fex();

	freq_data.max_freq = freq_data.extreme_freq;

	if(NULL != private_data.regulator_id)
	{
		private_data.regulator = regulator_get(NULL, private_data.regulator_id);
		if (IS_ERR_OR_NULL(private_data.regulator))
		{
			MALI_PRINT_ERROR(("Failed to get regulator!\n"));
			private_data.regulator = NULL;
		}
	}

	err = get_gpu_clk();
	err &= set_clk_freq(freq_data.normal_freq);
	err &= enable_gpu_clk();

	if(!err)
	{
		goto err_out;
	}

	MALI_PRINT(("Init Mali gpu clocks successfully\n"));
    return 0;

err_out:
	MALI_PRINT_ERROR(("Failed to init Mali gpu clocks!\n"));
	return -1;
}

/*
***************************************************************
 @Function   :mali_platform_device_init/
              aw_mali_platform_device_register
 @Description:Init the essential data of gpu
***************************************************************
*/
#ifdef CONFIG_MALI_DT
int mali_platform_device_init(struct platform_device *device)
#else
int aw_mali_platform_device_register(void)
#endif
{
	int err=0;
	struct platform_device *pdev;
	struct __fb_addr_para fb_addr_para = {0};

#ifdef CONFIG_MALI_DT
	pdev = device;
#else
	pdev = &mali_gpu_device;
#endif

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

#ifndef CONFIG_MALI_DT
	err = platform_device_add_resources(&mali_gpu_device, mali_gpu_resources, sizeof(mali_gpu_resources) / sizeof(mali_gpu_resources[0]));
	if(err)
	{
		MALI_PRINT_ERROR(("platform_device_add_resources failed!\n"));
		return err;
	}
#endif

	sunxi_get_fb_addr_para(&fb_addr_para);
	mali_gpu_data.fb_start = fb_addr_para.fb_paddr;
	mali_gpu_data.fb_size = fb_addr_para.fb_size;
	mali_gpu_data.shared_mem_size = totalram_pages * PAGE_SIZE; /* B */

	err = platform_device_add_data(pdev, &mali_gpu_data, sizeof(mali_gpu_data));
	if(err)
	{
		MALI_PRINT_ERROR(("platform_device_add_data failed!\n"));
		return err;
	}

#ifndef CONFIG_MALI_DT
	err = platform_device_register(&mali_gpu_device);
	if(err)
	{
		MALI_PRINT_ERROR(("platform_device_register failed!\n"));
		return err;
	}
#endif

	err = mali_platform_init(pdev);
	if(err)
	{
		return err;
	}

#if defined(CONFIG_PM_RUNTIME)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
#ifdef CONFIG_DEVFREQ_DRAM_FREQ_WITH_SOFT_NOTIFY
	pm_runtime_set_autosuspend_delay(&(pdev->dev), 1000);
#else
	pm_runtime_set_autosuspend_delay(&(pdev->dev), 2);
#endif
	pm_runtime_use_autosuspend(&(pdev->dev));
#endif
	pm_runtime_enable(&(pdev->dev));
#endif /* CONFIG_PM_RUNTIME */

	mutex_init(&private_data.lock);

	err = sysfs_create_group(&pdev->dev.kobj, &gpu_attribute_group);
	if (err)
	{
		kobject_put(&pdev->dev.kobj);
	}

#ifdef CONFIG_CPU_BUDGET_THERMAL
	register_budget_cooling_notifier(&gpu_throttle_notifier);
#endif

    return err;
}