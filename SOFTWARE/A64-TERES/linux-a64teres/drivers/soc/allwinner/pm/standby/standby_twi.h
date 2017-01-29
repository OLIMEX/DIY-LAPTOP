/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : standby_twi.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 15:22
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/

#ifndef __STANDBY_TWI_H__
#define __STANDBY_TWI_H__

enum twi_op_type_e{
    TWI_OP_RD,
    TWI_OP_WR,
};

extern __s32 standby_twi_init(int group);
extern __s32 standby_twi_exit(void);
extern __s32 twi_byte_rw(enum twi_op_type_e op, __u8 saddr, __u8 baddr, __u8 *data);

#endif  /* __STANDBY_TWI_H__ */

