/*
 * sunxi video front end sub device header file
 * Author:raymonxiu
 */

#ifndef __VFE__SUBDEV__H__
#define __VFE__SUBDEV__H__

#include <media/v4l2-subdev.h>

enum pmic_channel {
  IOVDD,
  DVDD,
  AVDD,
  AFVDD,
  FLVDD,
};

enum gpio_type {
  POWER_EN = 0,
  PWDN,
  RESET,
  AF_PWDN,
  FLASH_EN,
  FLASH_MODE,
  MCLK_PIN,
  MAX_GPIO_NUM,
};

enum gpio_fun {
	GPIO_INPUT = 0,
	GPIO_OUTPUT = 1,
	GPIO_DISABLE = 7,
};
enum on_off {
  OFF,
  ON,
};

enum standby_mode {
	NORM_STBY,
	POWER_OFF,
	HW_STBY,
	SW_STBY,
};

enum power_seq_cmd
{
  CSI_SUBDEV_INIT_FULL = 0x01,
  CSI_SUBDEV_INIT_SIMP = 0x02,
  CSI_SUBDEV_RST_ON = 0x03,
  CSI_SUBDEV_RST_OFF = 0x04,
  CSI_SUBDEV_RST_PUL = 0x05,
  CSI_SUBDEV_STBY_ON = 0x06,
  CSI_SUBDEV_STBY_OFF = 0x07,
  CSI_SUBDEV_PWR_ON = 0x08,
  CSI_SUBDEV_PWR_OFF = 0x09,
};

extern int vfe_set_pmu_channel(struct v4l2_subdev *sd, enum pmic_channel pmic_ch, enum on_off on_off);
extern int vfe_set_mclk(struct v4l2_subdev *sd, enum on_off on_off);
extern int vfe_set_mclk_freq(struct v4l2_subdev *sd, unsigned long freq);
extern int vfe_gpio_write(struct v4l2_subdev *sd, enum gpio_type gpio_type, unsigned int status);
extern int vfe_gpio_set_status(struct v4l2_subdev *sd, enum gpio_type gpio_type, unsigned int status);
extern void vfe_get_standby_mode(struct v4l2_subdev *sd, enum standby_mode *stby_mode);


#endif //__VFE__SUBDEV__H__
