/*
 * sunxi Camera core header file
 * Author:raymonxiu
 */
#ifndef __VFE__H__
#define __VFE__H__

#include <linux/types.h>
#include <linux/interrupt.h>
#include <media/sunxi_camera.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/pm_runtime.h>

#include <media/videobuf2-core.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-ctrls.h>

#include "platform_cfg.h"
#include "flash_light/flash.h"

#include "actuator/actuator.h"
#include "csi/bsp_csi.h"
#include "mipi_csi/bsp_mipi_csi.h"
#include "lib/bsp_isp.h"
#include "lib/bsp_isp_algo.h"
#include "vfe_subdev.h"
#include "vfe_os.h"

struct vfe_fmt {
	unsigned char               name[32];
	enum v4l2_mbus_pixelcode    bus_pix_code;
	unsigned int                fourcc;          /* v4l2 format id */
	enum v4l2_field             field;
	unsigned char               depth;
	unsigned char               planes_cnt;
};

struct vfe_coor {
	unsigned int    x1;
	unsigned int    y1;
	unsigned int    x2;
	unsigned int    y2;
};

/* buffer for one video frame */
struct vfe_buffer {
	struct vb2_buffer	vb;	
	struct list_head	list;
	struct vfe_fmt		*fmt;
};

struct vfe_dmaqueue {
	struct list_head  active;
	/* Counters to control fps rate */
	int               frame;
	int               ini_jiffies;
};

struct vfe_isp_stat_buf {
	unsigned int              id;
	struct list_head          queue;
	struct isp_stat_buffer    isp_stat_buf;
	void                      *paddr;  
	void						*dma_addr;  
};

struct vfe_isp_stat_buf_queue {
	struct list_head          active;
	struct list_head          locked;
	struct vfe_isp_stat_buf   isp_stat[MAX_ISP_STAT_BUF];
};

struct vfe_ctrl_para {
	unsigned int                            auto_wb;  //bool
	enum  v4l2_exposure_auto_type           exp_auto_mode;
	unsigned int                            auto_focus; //bool
	unsigned int                            exp_bias; //interger
	struct isp_h3a_coor_win 				af_coor[MAX_AF_WIN_NUM];//pointer
	struct isp_h3a_coor_win 				ae_coor[MAX_AE_WIN_NUM];//pointer
	unsigned int                            gsensor_rot; //interger
	unsigned int							prev_exp_line;
	unsigned int			                prev_ana_gain;  
	unsigned int			                prev_focus_pos;
	unsigned int			                prev_exp_gain;
};

struct vfe_regs {
	void __iomem      *csi_regs;
	struct resource   *csi_regs_res;
	void __iomem      *isp_regs;
	struct resource   *isp_regs_res;
	void __iomem      *isp_load_regs;
	void __iomem      *isp_saved_regs;
	void              *isp_load_regs_paddr;
	void              *isp_saved_regs_paddr;
	void              *isp_load_regs_dma_addr;
	void              *isp_saved_regs_dma_addr;
};

struct vfe_clk_freq {
	unsigned long			master_clk_freq;
	unsigned long			core_clk_freq;
	unsigned long			dphy_clk_freq;
};

enum vfe_regulator
{
	ENUM_ISP_REGULATOR,
	ENUM_CSI_REGULATOR,
};
enum vfe_sub_device_regulator
{
	ENUM_IOVDD,
	ENUM_AVDD,
	ENUM_DVDD,
	ENUM_AFVDD,
	ENUM_MAX_REGU,
};

struct vfe_power {
	/*power issue*/
	enum standby_mode  stby_mode; /* standby mode */
	struct regulator   *iovdd;    /* interface voltage source of sensor module */
	struct regulator   *avdd;     /* analog voltage source of sensor module */
	struct regulator   *dvdd;     /* core voltage source of sensor module */
	struct regulator   *afvdd;    /* vcm sink voltage source of sensor module */
	struct regulator   *flvdd;    /* flash led voltage source of sensor module */
	unsigned int		iovdd_vol; /* voltage of sensor module for interface */
	unsigned int		avdd_vol; /* voltage of sensor module for analog */
	unsigned int		dvdd_vol; /* voltage of sensor module for core */
	unsigned int		afvdd_vol; /* voltage of sensor module for vcm sink */	
	unsigned int		flvdd_vol; /* voltage of sensor module for flash led */
};
struct camera_instance {
	char name[I2C_NAME_SIZE];
	int i2c_addr;
	int sensor_type;
	int stdby_mode;
	int vflip;
	int hflip;
	char act_name[I2C_NAME_SIZE];
	int act_i2c_addr;	
	char isp_cfg_name[I2C_NAME_SIZE];
};

struct sensor_config_init {
	int used;
	int csi_sel;
	int device_sel;
	int twi_id;
	int power_settings_enable;
	int detect_sensor_num;
	char sub_power_str[ENUM_MAX_REGU][32];
	int sub_power_vol[ENUM_MAX_REGU];
	struct camera_instance camera_inst[MAX_SENSOR_DETECT_NUM];
};
struct ccm_config {
	struct v4l2_subdev      *sd;
	struct sensor_config_init *sensor_cfg_ini;
	char                    sensor_pos[32];
	char                    ccm[I2C_NAME_SIZE];
	char                    isp_cfg_name[I2C_NAME_SIZE];
	char                    iovdd_str[32];
	char                    avdd_str[32];
	char                    dvdd_str[32];
	char                    afvdd_str[32];
	unsigned int            twi_id;
	unsigned int            i2c_addr;
	unsigned int            vflip;
	unsigned int            hflip;
	unsigned int            vflip_thumb;
	unsigned int            hflip_thumb;
	unsigned int            is_isp_used;
	unsigned int            is_bayer_raw;
	struct vfe_gpio_cfg     gpio[MAX_GPIO_NUM];
	struct vfe_power        power;
	struct vfe_clk_freq	    clk_freq;
	int                     act_used;
	char                    act_name[I2C_NAME_SIZE];
	unsigned int            act_slave;
	struct actuator_ctrl_t  *act_ctrl;
	struct v4l2_subdev      *sd_act;
	int                     flash_used;
	__flash_driver_ic_type  flash_type;
	char                    flvdd_str[32];
};

struct sunxi_vip_platform_data {
	unsigned int vip_sel;
}; 

static LIST_HEAD(devlist);
struct vfe_dev {
	struct list_head        devlist;
	struct v4l2_device      v4l2_dev;
	struct v4l2_subdev      *sd;
	struct v4l2_subdev	  	*sd_act;
	struct v4l2_subdev 		*isp_sd;
	struct v4l2_subdev 		*csi_sd;
	struct v4l2_subdev 		*mipi_sd;
	struct v4l2_subdev 		*flash_sd;
	int                     flash_used;
	__flash_driver_ic_type 	flash_type;
	int                     vip_define_sensor_list;
	struct platform_device  *pdev;
	int            			id;
	spinlock_t              slock;
	struct mutex            stream_lock;
	struct delayed_work 	probe_work;
	/* various device info */
	struct video_device     *vfd;
	struct vfe_dmaqueue     vidq;
	struct vfe_isp_stat_buf_queue  isp_stat_bq;
	/* Several counters */
	unsigned                ms;
	unsigned long           jiffies;
	/* video capture */
	struct vb2_queue		vb_vidq;	
	struct mutex            buf_lock;	
	struct vb2_alloc_ctx 	*alloc_ctx;
	unsigned int            capture_mode;
	/*working state*/
	unsigned long           generating;
	unsigned long           opened;
	struct mutex            opened_lock;
	/* about system resource */
	int                     irq;  
#ifdef VFE_GPIO
	struct pinctrl		 	*pctrl;
	struct pinctrl_state 	*pctrl_state;
#endif  
	struct vfe_regs         regs;
	struct vfe_gpio_cfg	  	*gpio;

	struct vfe_power        *power;
	struct regulator   		*vfe_system_power[3];
	int vfe_sensor_power_cnt;
	struct vfe_clk_freq		*clk_freq;
	struct clk			  	*clock[CLK_NUM];
	struct clk			  	*clock_src[CLK_SRC_NUM];
	/* about vfe channel */ 
	unsigned int            cur_ch;
	/* about some global info*/
	unsigned int            first_flag;       /* indicate the first time triggering irq */
	long unsigned int       sec,usec;
	unsigned int            dev_qty;
	unsigned int            is_same_module;   /* the modules connected on the same bus are the same modle */
	unsigned int            input;
	enum v4l2_mbus_type     mbus_type;
	unsigned int            mipi_sel;
	unsigned int            vip_sel;
	unsigned int            isp_sel;
	struct ccm_config       ccm_cfg_content[MAX_INPUT_NUM];
	struct ccm_config       *ccm_cfg[MAX_INPUT_NUM];
	struct i2c_board_info   dev_sensor[MAX_INPUT_NUM];
	struct i2c_board_info   dev_act[MAX_INPUT_NUM];
	unsigned int   	   		device_valid_flag[MAX_INPUT_NUM];
	unsigned int            is_isp_used;
	unsigned int            is_bayer_raw;
	struct vfe_fmt          fmt;
	unsigned int            width;
	unsigned int            height;
	unsigned int            thumb_width;
	unsigned int            thumb_height;
	unsigned int            buf_byte_size;    /* including main and thumb buffer */
	unsigned int            buf_addr;         /* including main and thumb buffer */
	struct isp_frame_info   isp_frame_info;
	struct isp_init_para    isp_init_para;
	struct isp_table_addr   isp_tbl_addr[MAX_INPUT_NUM];
	struct vfe_mm  			isp_lut_tbl_buf_mm[MAX_INPUT_NUM];
	struct vfe_mm			isp_drc_tbl_buf_mm[MAX_INPUT_NUM];
	struct vfe_mm			isp_stat_buf_mm[MAX_ISP_STAT_BUF];
	struct isp_gen_settings  isp_gen_set[MAX_INPUT_NUM];
	struct isp_gen_settings  *isp_gen_set_pt; 
	struct mutex            isp_3a_result_mutex;
	struct isp_3a_result    isp_3a_result[MAX_INPUT_NUM];
	struct isp_3a_result    *isp_3a_result_pt;
	//struct tasklet_struct   isp_isr_bh_task;
	struct work_struct      isp_isr_bh_task;
	struct work_struct      isp_isr_set_sensor_task;
	struct vfe_ctrl_para    ctrl_para;
	struct flash_dev_info	*fl_dev_info;
	unsigned int			platform_id;
	unsigned int 			vfe_s_input_flag;
	struct v4l2_ctrl_handler  ctrl_handler;
	struct timer_list		timer_for_reset;
};

#endif  /* __VFE__H__ */


