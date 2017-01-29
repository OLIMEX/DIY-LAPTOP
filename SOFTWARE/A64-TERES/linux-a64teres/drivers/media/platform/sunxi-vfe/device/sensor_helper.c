/*
 * sensor helper source file
 *
 */

#include "sensor_helper.h"

/*
 * On most platforms, we'd rather do straight i2c I/O.
 */
int sensor_read(struct v4l2_subdev *sd, addr_type reg, data_type *value)
{
	int ret=0, cnt=0;
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);
#else
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
#endif
	
	ret = cci_read(sd,reg,value);
	while((ret != 0) && (cnt < 2))
	{
		ret = cci_read(sd,reg,value);
		cnt++;
	}
	if(cnt > 0)
		printk("%s sensor read retry=%d\n",cci_drv->name, cnt);

	return ret;
}
EXPORT_SYMBOL_GPL(sensor_read);

int sensor_write(struct v4l2_subdev *sd, addr_type reg, data_type value)
{
	int ret=0, cnt=0;
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);
#else
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
#endif

	ret = cci_write(sd,reg,value);
	while((ret != 0) && (cnt < 2))
	{
		ret = cci_write(sd,reg,value);
		cnt++;
	}	
	if(cnt > 0)
		printk("%s sensor write retry=%d\n",cci_drv->name, cnt);

	return ret;
}
EXPORT_SYMBOL_GPL(sensor_write);

/*
 * Write a list of register settings;
 */
int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int ret = 0, i = 0;
#ifdef USE_SPECIFIC_CCI
	struct cci_driver *cci_drv = v4l2_get_subdevdata(sd);
#else
	struct cci_driver *cci_drv = v4l2_get_subdev_hostdata(sd);
#endif

	if(!regs)
		return -EINVAL;

	while(i < array_size)
	{
		if(regs->addr == REG_DLY) {
			msleep(regs->data);
		} else {  
			ret = sensor_write(sd, regs->addr, regs->data);
			if(ret < 0)
				printk("%s sensor write array error!!\n",cci_drv->name);
		}
		i++;
		regs++;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(sensor_write_array);


//int sensor_power(struct v4l2_subdev *sd, int on)
//{
//	int ret;
//	//insure that clk_disable() and clk_enable() are called in pair 
//	//when calling CSI_SUBDEV_STBY_ON/OFF and CSI_SUBDEV_PWR_ON/OFF
//	ret = 0;
//	switch(on)
//	{
//		case CSI_SUBDEV_STBY_ON:
//			vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
//			usleep_range(10000,12000);
//			//make sure that no device can access i2c bus during sensor initial or power down
//			//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling cci_unlock
//			cci_lock(sd);
//			//standby on io
//			vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
//			//remember to unlock i2c adapter, so the device can access the i2c bus again
//			cci_unlock(sd);  
//			//inactive mclk after stadby in
//			vfe_set_mclk(sd,OFF);
//			break;
//		case CSI_SUBDEV_STBY_OFF:
//			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n"); 
//			//make sure that no device can access i2c bus during sensor initial or power down
//			//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling cci_unlock
//			cci_lock(sd);    
//			//active mclk before stadby out
//			vfe_set_mclk_freq(sd,MCLK);
//			vfe_set_mclk(sd,ON);
//			usleep_range(10000,12000);
//			//standby off io
//			vfe_gpio_write(sd,PWDN,CSI_STBY_OFF);
//			usleep_range(10000,12000);
//			//remember to unlock i2c adapter, so the device can access the i2c bus again
//			cci_unlock(sd);        
//			usleep_range(10000,12000);
//			break;
//		case CSI_SUBDEV_PWR_ON:
//			vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
//			//make sure that no device can access i2c bus during sensor initial or power down
//			//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling cci_unlock
//			cci_lock(sd);
//			//power on reset
//			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
//			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
//			//power down io
//			vfe_gpio_write(sd,PWDN,CSI_STBY_ON);
//			//reset on io
//			vfe_gpio_write(sd,RESET,CSI_RST_ON);
//			usleep_range(1000,1200);
//			//active mclk before power on
//			vfe_set_mclk_freq(sd,MCLK);
//			vfe_set_mclk(sd,ON);
//			usleep_range(10000,12000);
//			//power supply
//			vfe_gpio_write(sd,POWER_EN,CSI_PWR_ON);
//			vfe_set_pmu_channel(sd,IOVDD,ON);
//			vfe_set_pmu_channel(sd,AVDD,ON);
//			vfe_set_pmu_channel(sd,DVDD,ON);
//			vfe_set_pmu_channel(sd,AFVDD,ON);
//			//standby off io
//			vfe_gpio_write(sd,PWDN,CSI_STBY_OFF);
//			usleep_range(10000,12000);
//			//reset after power on
//			vfe_gpio_write(sd,RESET,CSI_RST_OFF);
//			usleep_range(30000,31000);
//			//remember to unlock i2c adapter, so the device can access the i2c bus again
//			cci_unlock(sd);  
//			break;
//		case CSI_SUBDEV_PWR_OFF:
//			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
//			//make sure that no device can access i2c bus during sensor initial or power down
//			//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling cci_unlock
//			cci_lock(sd);
//			//inactive mclk before power off
//			vfe_set_mclk(sd,OFF);
//			//power supply off
//			vfe_gpio_write(sd,POWER_EN,CSI_PWR_OFF);
//			vfe_set_pmu_channel(sd,AFVDD,OFF);
//			vfe_set_pmu_channel(sd,DVDD,OFF);
//			vfe_set_pmu_channel(sd,AVDD,OFF);
//			vfe_set_pmu_channel(sd,IOVDD,OFF);  
//			//standby and reset io
//			usleep_range(10000,12000);
//			vfe_gpio_write(sd,POWER_EN,CSI_STBY_OFF);
//			vfe_gpio_write(sd,RESET,CSI_RST_ON);
//			//set the io to hi-z
//			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
//			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
//			//remember to unlock i2c adapter, so the device can access the i2c bus again
//			cci_unlock(sd);  
//			break;
//		default:
//			return -EINVAL;
//	}
//	return 0;
//}
//EXPORT_SYMBOL_GPL(sensor_power);

