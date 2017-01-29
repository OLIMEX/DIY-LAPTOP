/*
 * Copyright (C) 2013 Allwinner Ltd., Hasan
 *
 * Hasan <sujiajia@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>



static int debug = 0;
module_param(debug, int,0);
MODULE_PARM_DESC(schw_debug, "schw Enable debug");

#undef dprintk
#define dprintk(format, arg...)        \
       do { \
               if (debug) \
               printk(KERN_DEBUG "%s: " format, \
                               __FUNCTION__, ## arg);  \
       }while(0)

#define fprintk(format, arg...)        \
       do { \
               if (debug) \
               printk(format, ## arg);    \
       }while(0)


#define derr(format, arg...)        \
       do { \
               if (debug) \
               printk(KERN_ERR "%s: " format, \
                               __FUNCTION__, ## arg);  \
       }while(0)
#define SCHW_READ 0xfe
#define SCHW_WRITE 0xef
#define SCHW_FLUSH 0Xff
#define SCHW_CRYPT 0x5a
#define SCHW_NONE 0x00

#define SCHW_DEV_MINOR 111
#define SCHW_IOCTL_BASE 'C'
#define SCHW_IOCTL_OPRA 'S'

#define    SCHW_GET_SLOT            _IOR(SCHW_IOCTL_BASE, 1, int)
#define    SCHW_SET_SLOT            _IOR(SCHW_IOCTL_BASE, 2, unsigned char)

#define    SCHW_SECURE_STORE         _IOW(SCHW_IOCTL_OPRA, 1, unsigned char)
#define    SCHW_SECURE_PULL         _IOW(SCHW_IOCTL_OPRA, 2, int)
#define    SCHW_SECURE_LEN         _IOW(SCHW_IOCTL_OPRA, 3, int)
#define    SCHW_SECURE_CLEAR         _IOW(SCHW_IOCTL_OPRA, 4, unsigned char)

struct map_slot {
       struct list_head slist;
       unsigned char fname[50];
       int slot_id;
};

typedef struct map_slot mslot_st;
typedef struct map_slot* mslot_pt;


struct fcrypt {
       unsigned char *bpt;
       mslot_pt slot;
       int resh_id;
       int bpt_len;
       int encrypted;
       int cp_src;
       int cry_frm;
       int status;
       struct task_struct *tsk;
       struct completion tsk_started;
       struct completion tsk_end;
       spinlock_t task_state_lock;
       struct mutex mutex;
       struct list_head slot_list;

};

typedef struct fcrypt fcry_st;
typedef struct fcrypt* fcry_pt;


void schw_file_dump(const unsigned char *pr, int size);


struct schw_key_result {
       int err;
       struct completion completion;
};

struct schw_crypt_req_data {
       u8 iv[16];
       struct schw_key_result result;
       struct scatterlist src;
       struct scatterlist dst;
};


int schw_storage_encrypt(const void *key, int key_len,
               void *dst, size_t *dst_len,
               const void *src, size_t src_len);

int schw_storage_decrypt(const void *key, int key_len,
               void *dst, size_t *dst_len,
               const void *src, size_t src_len);

int schw_storage_secure(fcry_pt spt);

mslot_pt schw_mlloc_group(int creat);

int schw_update_mapblk(fcry_pt fpt);

int schw_operate_storage(fcry_pt fpt);

int schw_recover_storage(fcry_pt fpt);

int schw_flush_storage(fcry_pt fcr, int id);
int schw_slot_dequeue(fcry_pt fpt);

int schw_slot_enqueue(fcry_pt fpt);

int schw_slot_fluqueue(fcry_pt fpt);

int schw_slot_search(fcry_pt fpt);
