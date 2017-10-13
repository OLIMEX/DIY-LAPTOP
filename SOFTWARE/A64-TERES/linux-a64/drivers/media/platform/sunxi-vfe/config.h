/*
 * header file
 * set/get config from sysconfig and ini/bin file
 * Author: raymonxiu
 * 
 */
 
#ifndef __CONFIG__H__
#define __CONFIG__H__

#include "vfe.h"
#include "utility/cfg_op.h"

int fetch_config(struct vfe_dev *dev);
int read_ini_info(struct vfe_dev *dev,int isp_id, char *main_path);

#endif //__CONFIG__H__