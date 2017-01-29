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
#include "aw_schw_lib.h"


static fcry_pt nfcr = NULL;

static int schw_lisener_loop(void* context)
{
       fcry_pt fcpt = NULL;
       int ret = 0;
       fcpt = (fcry_pt)context;
       do {
               if (kthread_should_stop())
                       break;
               wait_for_completion(&fcpt->tsk_started);

               dprintk("::openrate secure storage!\n");
               mutex_lock(&fcpt->mutex);
               if ((fcpt->cp_src == SCHW_WRITE) ||
                               (fcpt->cp_src == SCHW_READ)) {
                       ret = schw_operate_storage(fcpt);
                       if (ret < 0)
                               derr("schw secure storage fail !\n");
               } else if (fcpt->cp_src == SCHW_CRYPT) {
                       ret = schw_update_mapblk(fcpt);
                       if (ret < 0)
                               derr("schw secure storage fail !\n");
               }
               mutex_unlock(&fcpt->mutex);
               if ((fcpt->cp_src == SCHW_WRITE) ||
                               (fcpt->cp_src == SCHW_READ) ||
                               (fcpt->cp_src == SCHW_CRYPT)) {
                       complete(&fcpt->tsk_end);
               }
       }while (!kthread_should_stop());

       return 0;
}

static ssize_t schw_dev_write(struct file *filp,const char __user *buf,
               size_t count, loff_t *f_pos)
{
       fcry_pt fcr = NULL;
       int id = 0;
       int ret = -EFAULT;

       dprintk("Enter count %ld!\n",count);
       fcr = filp->private_data;
       if (fcr == NULL) {
               derr("file private null!\n");
               return -1;
       }

       mutex_lock(&fcr->mutex);

       fcr->bpt_len = count;
       fcr->cp_src =SCHW_WRITE;
       fcr->bpt = (unsigned char *)kmalloc(count, GFP_KERNEL);
       if (!fcr->bpt) {
               mutex_unlock(&fcr->mutex);
               return -ENOMEM;
       }
       memset(fcr->bpt, 0, count);

       if(copy_from_user((void*)fcr->bpt, buf, count))
               goto err_out;

       fcr->encrypted = 1;
       if (schw_storage_secure(fcr) < 0)
               goto err_out;

       mutex_unlock(&fcr->mutex);

       complete(&fcr->tsk_started);
       wait_for_completion(&fcr->tsk_end);

       mutex_lock(&fcr->mutex);
       if (fcr->status < 0)
               goto err_out;

       id = fcr->slot->slot_id;
       fcr->slot = NULL;
       kfree((void*)fcr->bpt);
       fcr->bpt = NULL;
       fcr->bpt_len = 0;
       fcr->cp_src = SCHW_NONE;
       fcr->status = -1;
       mutex_unlock(&fcr->mutex);

       if (schw_flush_storage(fcr,id) == 0) {
               fcr->status = -1;
               ret = 0;
       }

       dprintk("exit ret %d!\n",ret);
       return (ret==0)?count:ret;

err_out:
       fcr->slot = NULL;
       kfree((void*)fcr->bpt);
       fcr->bpt = NULL;
       fcr->bpt_len = 0;
       fcr->cp_src = SCHW_NONE;
       mutex_unlock(&fcr->mutex);
       dprintk("exit err ret %d!\n",ret);
       return ret;
}

static ssize_t schw_dev_read(struct file *filp, char __user *buf,
               size_t count, loff_t *f_pos)
{
       fcry_pt fcr = NULL;
       int cryp_cnt = 0;
       int ret = -EFAULT;
       dprintk("Enter ! count %ld\n",count);
       fcr = filp->private_data;
       if (fcr == NULL) {
               derr("file private null!\n");
               return -EFAULT;
       }

       mutex_lock(&fcr->mutex);

       cryp_cnt = count + 100;
       fcr->bpt_len = cryp_cnt;
       fcr->cp_src = SCHW_READ;
       fcr->bpt = kmalloc(cryp_cnt, GFP_KERNEL);
       if (!fcr->bpt) {
               mutex_unlock(&fcr->mutex);
               return -ENOMEM;
       }
       memset(fcr->bpt, 0, cryp_cnt);
       if (schw_slot_search(fcr) < 0)
               goto err_out;

       mutex_unlock(&fcr->mutex);

       complete(&fcr->tsk_started);
       wait_for_completion(&fcr->tsk_end);

       mutex_lock(&fcr->mutex);

       if (fcr->status < 0)
               goto err_out;

       fcr->encrypted = 0;
       if (schw_storage_secure(fcr) < 0)
               goto err_out;

       if (copy_to_user((char __user *)buf, (void*)fcr->bpt,
                               (unsigned long)fcr->bpt_len))
               goto err_out;

       ret = 0;

err_out:
       kfree(fcr->bpt);
       fcr->bpt = NULL;
       fcr->slot = NULL;
       fcr->bpt_len = 0;
       fcr->resh_id = -1;
       fcr->cp_src = SCHW_NONE;
       mutex_unlock(&fcr->mutex);
       dprintk("exit ret %d !\n",ret);
       return (ret==0)?count:ret;
}

static long schw_dev_ioctl(struct file *filp, unsigned int cmd,
               unsigned long arg)
{
       int ret = -ENOTTY;
       fcry_pt fcr = NULL;
       int id = -1;
       size_t glen = -1;

       dprintk("Enter !\n");
       fcr = filp->private_data;
       if (fcr == NULL) {
               derr("file private null!\n");
               return -1;
       }
       switch (cmd) {
               case SCHW_GET_SLOT:
                       dprintk("SCHW_GET_SLOT: \n");
                       schw_slot_enqueue(fcr);
                       id = fcr->slot->slot_id;
                       if (put_user(id, (int __user *) arg))
                               return -EFAULT;

                       ret = 0;
                       break;

               case SCHW_SECURE_LEN:
                       if(copy_from_user((void*)&glen, (void __user *)arg,
                                               (unsigned long)sizeof(glen)))
                               return -EFAULT;
                       dprintk("SCHW_SECURE_LEN: \n");
                       fcr->bpt_len = glen;
                       fcr->bpt = kmalloc(glen, GFP_KERNEL);
                       if (!fcr->bpt)
                               return -ENOMEM;
                       memset(fcr->bpt, 0, glen);
                       put_user(1, (int *)arg);
                       ret = 0;
                       break;

               case SCHW_SECURE_STORE:
                       dprintk("SCHW_SECURE_STORE: \n");

                       ret = 0;
                       break;

               case SCHW_SECURE_PULL:
                       dprintk("SCHW_SECURE_PULL: \n");

                       ret = 0;
                       break;

               case SCHW_SET_SLOT:
                       dprintk("SCHW_SET_SLOT: \n");
                       if(copy_from_user((void*)(&fcr->resh_id),(void __user *)arg,
                                               (unsigned long)sizeof(fcr->resh_id)))
                               return -EFAULT;
                       ret = 0;
                       break;

               case SCHW_SECURE_CLEAR:
                       ret = 0;
                       break;
       }
       dprintk("Exit ret%d.....!\n",ret);
       return ret;
}

static int schw_dev_release(struct inode *inode, struct file *filp)
{
       dprintk("Enter\n");


       return 0;
}


static int schw_dev_open(struct inode *inode, struct file *filp)
{
       dprintk("Ebter\n");

       filp->private_data = nfcr;
       return(0);
}

static const struct file_operations schw_dev_fops = {
       .owner = THIS_MODULE,
       .llseek = no_llseek,
       .write = schw_dev_write,
       .read  = schw_dev_read,
       .unlocked_ioctl = schw_dev_ioctl,
       .open = schw_dev_open,
       .release = schw_dev_release,
};


static struct miscdevice schw_crypto_miscdev = {
       .minor = SCHW_DEV_MINOR,
       .name = "scdev",
       .fops = &schw_dev_fops,
};

static int __init schwdev_init(void)
{
       int rc;

       dprintk("(%p)\n", schwdev_init);
       rc = misc_register(&schw_crypto_miscdev);
       if (rc) {
               derr(" registration dev failed!\n");
               return(rc);
       }
       nfcr = kmalloc(sizeof(*nfcr), GFP_KERNEL);
       if (!nfcr) {
               derr(" - Malloc failed\n");
               return(-ENOMEM);
       }
       memset(nfcr, 0, sizeof(*nfcr));

       init_completion(&nfcr->tsk_started);
       init_completion(&nfcr->tsk_end);
       mutex_init(&nfcr->mutex);
       INIT_LIST_HEAD(&nfcr->slot_list);

       nfcr->tsk = kthread_run(schw_lisener_loop, nfcr, "schw");
       if (IS_ERR(nfcr->tsk)) {
               derr("Tsk request fail!\n");
               return (-EINVAL);
       }

       if (schw_slot_fluqueue(nfcr) < 0) {
               derr("Flush slot fail!\n");
               return (-EINVAL);
       }

       return(0);
}

static void __exit schwdev_exit(void)
{
       misc_deregister(&schw_crypto_miscdev);
       complete(&nfcr->tsk_started);
       kthread_stop(nfcr->tsk);
       schw_slot_dequeue(nfcr);
       kfree(nfcr);
}

module_init(schwdev_init);
module_exit(schwdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pd3 hasan-Sj <sujiajia@allsoftwinnertech.com>");
MODULE_DESCRIPTION("schw_crypto_miscdev (usr interface to create session with secure)");
