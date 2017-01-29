/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : mem_usb.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-31 15:17
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __MEM_USB_H__
#define __MEM_USB_H__

extern __s32 mem_usb_init(void);
extern __s32 mem_usb_exit(void);
extern __s32 mem_query_usb_event(void);
extern __s32 mem_is_usb_status_change(__u32 port);
#endif  /* __MEM_USB_H__ */

