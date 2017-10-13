/*
 * sunxi sensor header file
 * Author:raymonxiu
 */
#ifndef __CAMERA_CFG_H__
#define __CAMERA_CFG_H__

typedef enum tag_CAMERA_IO_CMD {
	GET_CURRENT_WIN_CFG,
	SET_FPS,
	SET_FLASH_CTRL,
	ISP_SET_EXP_GAIN,
	GET_SENSOR_EXIF,
	SET_AUTO_FOCUS_WIN,
	SET_AUTO_EXPOSURE_WIN,
	GET_FLASH_FLAG,
}__camera_cmd_t;

struct sensor_exp_gain {
	int exp_val;
	int gain_val;	
};

struct sensor_exif_attribute {
	__u32 exposure_time_num;
	__u32 exposure_time_den;
	__u32 fnumber;
	__u32 focal_length;
	__u32 iso_speed;
	__u32 flash_fire;
	__u32 brightness;
};

struct sensor_win_size {
  int width;
  int height;
  unsigned int hoffset;     //receive hoffset from sensor output
  unsigned int voffset;     //receive voffset from sensor output
  unsigned int hts;         //h size of timing, unit: pclk
  unsigned int vts;         //v size of timing, unit: line
  unsigned int pclk;        //pixel clock in Hz
  unsigned int mipi_bps;		//mipi clock in bps, fill this if config for mipi,
  unsigned int fps_fixed;   //fps mode 1=fixed fps
                            //N=varied fps to 1/N of org fps
  unsigned int bin_factor;  //binning factor
  unsigned int intg_min;    //integration min, unit: line, Q4
  unsigned int intg_max;    //integration max, unit: line, Q4
  unsigned int gain_min;    //sensor gain min, Q4
  unsigned int gain_max;    //sensor gain max, Q4
  int width_input;
  int height_input;

  void * regs; /* Regs to tweak */
  int regs_size;
  int (*set_size) (struct v4l2_subdev *sd);
/* h/vref stuff */
};


#endif //__CAMERA_CFG_H__