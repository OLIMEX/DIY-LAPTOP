/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2016 Joachim Damm
 * Copyright (C) 2016 Kamil Trzcinski
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file hdmi_cec.c
 *
 * @brief HDMI CEC system initialization and file operation implementation
 *
 * @ingroup HDMI
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>           /* for struct file_operations */
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/fsl_devices.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <asm/sizes.h>
#include <linux/module.h>
#include <linux/bitrev.h>
#include <linux/kthread.h>
#include <linux/console.h>
#include <linux/types.h>

#include "dw_hdmi.h"
#include "hdmi_cec.h"
#include "hdmi_edid.h"
#include "drv_hdmi_i.h"
#include "hdmi_bsp.h"
#include "hdmi_core.h"

#define MAX_MESSAGE_LEN               17

#define MESSAGE_TYPE_RECEIVE_SUCCESS  1
#define MESSAGE_TYPE_NOACK            2
#define MESSAGE_TYPE_DISCONNECTED     3
#define MESSAGE_TYPE_CONNECTED        4
#define MESSAGE_TYPE_SEND_SUCCESS     5

#define SENT_TRIES 2

enum {
  CEC_TX_AVAIL,
  CEC_TX_INPROGRESS,
  CEC_TX_INPROGRESS_SENT,
  CEC_TX_NACK,
  CEC_TX_ERROR,
  CEC_TX_DONE,
  CEC_TX_MAX
};

static const char *cec_tx_answer[CEC_TX_MAX] = {
  "CEC_TX_AVAIL",
  "CEC_TX_INPROGRESS",
  "CEC_TX_INPROGRESS_SENT",
  "CEC_TX_NACK",
  "CEC_TX_ERROR",
  "CEC_TX_DONE"
};

struct hdmi_cec_priv {
    u8 Logical_address;
    bool cec_state;
    u8 last_msg[MAX_MESSAGE_LEN];
    u8 msg_len;
    int tx_answer;
    int sent_tries;
    struct mutex lock;
};

struct hdmi_cec_event {
    int event_type;
    int msg_len;
    u8 msg[MAX_MESSAGE_LEN];
    struct list_head list;
};

static LIST_HEAD(head);

static int hdmi_cec_major;
static struct class *hdmi_cec_class;
static struct hdmi_cec_priv hdmi_cec_data;
static u8 open_count;
static wait_queue_head_t hdmi_cec_queue;
static wait_queue_head_t tx_cec_queue;
static struct task_struct * cec_task = NULL;

static int tx_reg[16] = {HDMI_CEC_TX_DATA0, HDMI_CEC_TX_DATA1, HDMI_CEC_TX_DATA2,\
            HDMI_CEC_TX_DATA3, HDMI_CEC_TX_DATA4, HDMI_CEC_TX_DATA5,\
            HDMI_CEC_TX_DATA6, HDMI_CEC_TX_DATA7, HDMI_CEC_TX_DATA8,\
            HDMI_CEC_TX_DATA9, HDMI_CEC_TX_DATA10, HDMI_CEC_TX_DATA11,\
            HDMI_CEC_TX_DATA12, HDMI_CEC_TX_DATA13, HDMI_CEC_TX_DATA14,\
            HDMI_CEC_TX_DATA15};

static int rx_reg[16] = {HDMI_CEC_RX_DATA0, HDMI_CEC_RX_DATA1, HDMI_CEC_RX_DATA2,\
            HDMI_CEC_RX_DATA3, HDMI_CEC_RX_DATA4, HDMI_CEC_RX_DATA5,\
            HDMI_CEC_RX_DATA6, HDMI_CEC_RX_DATA7, HDMI_CEC_RX_DATA8,\
            HDMI_CEC_RX_DATA9, HDMI_CEC_RX_DATA10, HDMI_CEC_RX_DATA11,\
            HDMI_CEC_RX_DATA12, HDMI_CEC_RX_DATA13, HDMI_CEC_RX_DATA14,\
            HDMI_CEC_RX_DATA15};

static u8 hdmi_read(unsigned int addr)
{
    return __raw_readb((volatile void *)((unsigned long long)hdmi_base_addr + addr));
}

static void hdmi_write(unsigned int addr, unsigned char data)
{
    __raw_writeb(data, (volatile void *)((unsigned long long)hdmi_base_addr + addr));
}

static void hdmi_cec_read_unlock(void)
{
  mutex_lock(&hdmi_cec_data.lock);
  hdmi_write(0x10010u, 0x45u);
  hdmi_write(0x10011u, 0x45u);
  hdmi_write(0x10012u, 0x52u);
  hdmi_write(0x10013u, 0x54u);
}

static void hdmi_cec_read_lock(void)
{
  hdmi_write(0x10010u, 0x52u);
  hdmi_write(0x10011u, 0x54u);
  hdmi_write(0x10012u, 0x41u);
  hdmi_write(0x10013u, 0x57u);
  mutex_unlock(&hdmi_cec_data.lock);
}

static void hdmi_cec_configure(void)
{
  u8 clkdis, val;

  val = HDMI_IH_CEC_STAT0_ERROR_INIT | HDMI_IH_CEC_STAT0_NACK | HDMI_IH_CEC_STAT0_EOM | HDMI_IH_CEC_STAT0_DONE;
  hdmi_write(HDMI_CEC_POLARITY, val);
  val = HDMI_IH_CEC_STAT0_WAKEUP | HDMI_IH_CEC_STAT0_ERROR_FOLL | HDMI_IH_CEC_STAT0_ARB_LOST;
  hdmi_write(HDMI_CEC_MASK, val);
  hdmi_write(HDMI_IH_MUTE_CEC_STAT0, 0x7f);

  hdmi_cec_read_unlock();
  clkdis = hdmi_read(HDMI_MC_CLKDIS);
  clkdis &= ~HDMI_MC_CLKDIS_CECCLK_DISABLE;
  hdmi_write(HDMI_MC_CLKDIS, clkdis);
  hdmi_cec_read_lock();
}

static void hdmi_cec_start_sending(void)
{
  hdmi_write(0x1003Cu, 0x0);
  hdmi_write(HDMI_CEC_CTRL, 0x2);
}

static void hdmi_cec_start_receiving(void)
{
  hdmi_write(0x1003Cu, 0x0);
  hdmi_write(HDMI_CEC_CTRL, 0x0);
}

static void hdmi_cec_message(int type, const unsigned char *buf, size_t length)
{
  struct hdmi_cec_event *event;

  event = vmalloc(sizeof(struct hdmi_cec_event));
  if (NULL == event) {
      pr_err("%s: Not enough memory!\n", __func__);
      return;
  }
  memset(event, 0, sizeof(struct hdmi_cec_event));
  event->event_type = type;
  event->msg_len = length;
  memcpy(event->msg, buf, length);
  mutex_lock(&hdmi_cec_data.lock);
  list_add_tail(&event->list, &head);
  mutex_unlock(&hdmi_cec_data.lock);
  wake_up(&hdmi_cec_queue);
}

static int hdmi_cec_thread(void *parg);

static void hdmi_cec_resume(void)
{
  if (cec_task == NULL) {
    cec_task = kthread_run(hdmi_cec_thread, NULL, "hdmi_cec_thread");
  }
}

static void hdmi_cec_suspend(void)
{
  if (cec_task != NULL) {
    kthread_stop(cec_task);
    cec_task = NULL;
  }
}

static int hdmi_cec_set_logical_address(void)
{
  u8 cec_l_addr_l = 0, cec_l_addr_h = 0;

  if (hdmi_cec_data.Logical_address <= 7) {
      cec_l_addr_l = 1 << hdmi_cec_data.Logical_address;
      cec_l_addr_h = 0;
  } else if (hdmi_cec_data.Logical_address > 7 && hdmi_cec_data.Logical_address <= 15) {
      cec_l_addr_l = 0;
      cec_l_addr_h = 1 << (hdmi_cec_data.Logical_address - 8);
  } else{
      return -EINVAL;
  }
  hdmi_write(HDMI_CEC_ADDR_L, cec_l_addr_l);
  hdmi_write(HDMI_CEC_ADDR_H, cec_l_addr_h);
  return 0;
}

static int sunxi_hdmi_notify(struct notifier_block *nb,
                             unsigned long code, void *unused)
{
    if (!open_count) {
      return NOTIFY_DONE;
    }
    switch (code) {
    case 0x00: // Unplug
      pr_info("[CEC]HDMI link disconnected\n");
      hdmi_cec_message(MESSAGE_TYPE_DISCONNECTED, NULL, 0);
      break;
    case 0x04: // Plug
      pr_info("[CEC]HDMI link connected\n");
      break;
    case 0x05: // reinit done
      pr_info("[CEC]HDMI reinitialized\n");
      hdmi_cec_configure();
      hdmi_cec_start_receiving();
      hdmi_cec_message(MESSAGE_TYPE_CONNECTED, NULL, 0);
      break;
    }
    return NOTIFY_DONE;
}

static struct notifier_block sunxi_hdmi_nb = {
  .notifier_call = sunxi_hdmi_notify,
};

/*!
 * @brief open function for cec file operation
 *
 * @return  0 on success or negative error code on error
 */
static int hdmi_cec_open(struct inode *inode, struct file *filp)
{
    mutex_lock(&hdmi_cec_data.lock);
    if (open_count) {
        mutex_unlock(&hdmi_cec_data.lock);
        return -EBUSY;
    }
    open_count = 1;
    filp->private_data = (void *)(&hdmi_cec_data);
    hdmi_cec_data.Logical_address = 15;
    hdmi_cec_data.cec_state = false;
    mutex_unlock(&hdmi_cec_data.lock);
    return 0;
}

static ssize_t hdmi_cec_read(struct file *file, char __user *buf, size_t count,
                loff_t *ppos)
{
    struct hdmi_cec_event *event = NULL;

    if (!open_count) {
        return -ENODEV;
    }
    mutex_lock(&hdmi_cec_data.lock);
    if (false == hdmi_cec_data.cec_state) {
        mutex_unlock(&hdmi_cec_data.lock);
        return -EACCES;
    }

    if (list_empty(&head)) {
        if (file->f_flags & O_NONBLOCK) {
            mutex_unlock(&hdmi_cec_data.lock);
            return -EAGAIN;
        } else {
            do {
                mutex_unlock(&hdmi_cec_data.lock);
                if (wait_event_interruptible(hdmi_cec_queue, (!list_empty(&head)))) {
                    return -ERESTARTSYS;
                }
                mutex_lock(&hdmi_cec_data.lock);
            } while (list_empty(&head));
        }
    }

    event = list_first_entry(&head, struct hdmi_cec_event, list);
    list_del(&event->list);
    mutex_unlock(&hdmi_cec_data.lock);
    if (copy_to_user(buf, event,
             sizeof(struct hdmi_cec_event) - sizeof(struct list_head))) {
        vfree(event);
        return -EFAULT;
    }
    vfree(event);
    return (sizeof(struct hdmi_cec_event) - sizeof(struct list_head));
}

static ssize_t hdmi_cec_write(struct file *file, const char __user *buf,
                 size_t count, loff_t *ppos)
{
    int ret = 0;
    u8 msg[MAX_MESSAGE_LEN];

    if (!open_count) {
        return -ENODEV;
    }
    if (count > MAX_MESSAGE_LEN) {
        return -EINVAL;
    }

    memset(&msg, 0, MAX_MESSAGE_LEN);
    ret = copy_from_user(&msg, buf, count);
    if (ret) {
        return -EACCES;
    }

    mutex_lock(&hdmi_cec_data.lock);
    if (false == hdmi_cec_data.cec_state) {
        mutex_unlock(&hdmi_cec_data.lock);
        return -EACCES;
    }
    if (hdmi_cec_data.tx_answer != CEC_TX_AVAIL) {
        mutex_unlock(&hdmi_cec_data.lock);
        return -EBUSY;
    }
    memcpy(hdmi_cec_data.last_msg, msg, count);
    hdmi_cec_data.msg_len = count;
    hdmi_cec_data.sent_tries = SENT_TRIES;
    hdmi_cec_data.tx_answer = CEC_TX_INPROGRESS;
    mutex_unlock(&hdmi_cec_data.lock);

    ret = wait_event_interruptible_timeout(tx_cec_queue, hdmi_cec_data.tx_answer > CEC_TX_INPROGRESS_SENT, HZ);
    if (ret < 0) {
        ret = -ERESTARTSYS;
        goto tx_out;
    }

    if (hdmi_cec_data.tx_answer == CEC_TX_DONE) {
      ret = count;
    } else if(hdmi_cec_data.tx_answer == CEC_TX_NACK) {
      ret = -EIO;
    } else {
      ret = -EACCES;
    }

tx_out:
    hdmi_cec_data.tx_answer = CEC_TX_AVAIL;
    return ret;
}

/*!
 * @brief IO ctrl function for cec file operation
 * @param cmd IO ctrl command
 * @return  0 on success or negative error code on error
 */
static long hdmi_cec_ioctl(struct file *filp, u_int cmd,
             u_long arg)
{
    int ret = 0, status = 0;
    if (!open_count)
        return -ENODEV;

    switch (cmd) {
    case HDMICEC_IOC_SETLOGICALADDRESS:
        mutex_lock(&hdmi_cec_data.lock);
        if (false == hdmi_cec_data.cec_state) {
            mutex_unlock(&hdmi_cec_data.lock);
            return -EACCES;
        }
        hdmi_cec_data.Logical_address = (u8)arg;
        pr_err("[HDMI_CEC] set logical_address=%lu\n", arg);
        mutex_unlock(&hdmi_cec_data.lock);
        break;
    case HDMICEC_IOC_STARTDEVICE:
        mutex_lock(&hdmi_cec_data.lock);
        hdmi_cec_data.cec_state = true;
        mutex_unlock(&hdmi_cec_data.lock);
        hdmi_cec_resume();
        break;
    case HDMICEC_IOC_STOPDEVICE:
        mutex_lock(&hdmi_cec_data.lock);
        hdmi_cec_data.cec_state = false;
        mutex_unlock(&hdmi_cec_data.lock);
        hdmi_cec_suspend();
        break;
    case HDMICEC_IOC_GETPHYADDRESS:
        status = copy_to_user((void __user *)arg,
                     &cec_phy_addr,
                     sizeof(u32));
        if (status)
            ret = -EFAULT;
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

/*!
 * @brief Release function for cec file operation
 * @return  0 on success or negative error code on error
 */
static int hdmi_cec_release(struct inode *inode, struct file *filp)
{
    mutex_lock(&hdmi_cec_data.lock);
    if (open_count) {
        open_count = 0;
        hdmi_cec_data.cec_state = false;
        hdmi_cec_data.Logical_address = 15;
    }
    mutex_unlock(&hdmi_cec_data.lock);

    hdmi_cec_suspend();

    return 0;
}

static unsigned int hdmi_cec_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;

    poll_wait(file, &hdmi_cec_queue, wait);

    /* Always writable */
    mask =  (POLLOUT | POLLWRNORM);
    mutex_lock(&hdmi_cec_data.lock);
    if (!list_empty(&head))
            mask |= (POLLIN | POLLRDNORM);
    mutex_unlock(&hdmi_cec_data.lock);
    return mask;
}

const struct file_operations hdmi_cec_fops = {
    .owner = THIS_MODULE,
    .read = hdmi_cec_read,
    .write = hdmi_cec_write,
    .open = hdmi_cec_open,
    .unlocked_ioctl = hdmi_cec_ioctl,
    .release = hdmi_cec_release,
    .poll = hdmi_cec_poll,
};

static int hdmi_cec_get_msg(unsigned char *msg)
{
  int ret = -1;

  hdmi_cec_read_unlock();
  hdmi_cec_set_logical_address();

  if(hdmi_read(HDMI_CEC_LOCK) & 1) {
    int i;

    ret = hdmi_read(HDMI_CEC_RX_CNT);
    for(i = 0; i < ret; ++i) {
      msg[i] = hdmi_read(rx_reg[i]);
    }
    hdmi_write(HDMI_CEC_LOCK, 0);
  }

  hdmi_cec_read_lock();
  return ret;
}

static u8 hdmi_cec_status(void)
{
  u8 stat;

  hdmi_cec_read_unlock();
  stat = hdmi_read(HDMI_IH_CEC_STAT0);
  hdmi_write(HDMI_IH_CEC_STAT0, stat);
  hdmi_cec_read_lock();
  return stat;
}

static int hdmi_cec_receive(void)
{
  unsigned char buf[32];
  int ret;

  memset(buf, 0, sizeof(buf));
  ret = hdmi_cec_get_msg(buf);
  if (ret >= 0) {
    printk(KERN_INFO "hdmi_cec_receive: buf=%02x %02x %02x %02x msg_len=%d\n",
      buf[0], buf[1], buf[2], buf[3], ret);
    hdmi_cec_message(MESSAGE_TYPE_RECEIVE_SUCCESS, buf, ret);
  }
  return ret;
}

static void hdmi_cec_send(void)
{
  int i;
  u8 val;

  if (hdmi_cec_data.tx_answer != CEC_TX_INPROGRESS) {
    return;
  }

  for (i = 0; i < hdmi_cec_data.msg_len; i++)
    hdmi_write(tx_reg[i], hdmi_cec_data.last_msg[i]);
  hdmi_write(HDMI_CEC_TX_CNT, hdmi_cec_data.msg_len);

  hdmi_cec_read_unlock();
  val = hdmi_read(HDMI_CEC_CTRL);
  val |= 0x01;
  hdmi_write(HDMI_CEC_CTRL, val);
  hdmi_cec_data.tx_answer = CEC_TX_INPROGRESS_SENT;
  hdmi_cec_read_lock();
}

static int hdmi_cec_send_retry()
{
  u8 val;

  if (hdmi_cec_data.tx_answer != CEC_TX_INPROGRESS_SENT) {
    return -1;
  }

  hdmi_cec_data.sent_tries--;

  if (hdmi_cec_data.sent_tries > 0) {
    hdmi_cec_read_unlock();
    val = hdmi_read(HDMI_CEC_CTRL);
    val |= 0x01;
    hdmi_write(HDMI_CEC_CTRL, val);
    hdmi_cec_read_lock();
    return 0;
  }
  return -1;
}

static void hdmi_cec_finish_send(int tx_answer)
{
  if (hdmi_cec_data.tx_answer != CEC_TX_INPROGRESS_SENT) {
    return;
  }

  printk(KERN_INFO "hdmi_cec_finish_send: buf=%02x %02x %02x %02x msg_len=%d tx_answer=%s\n",
    hdmi_cec_data.last_msg[0], hdmi_cec_data.last_msg[1],
    hdmi_cec_data.last_msg[2], hdmi_cec_data.last_msg[3],
    hdmi_cec_data.msg_len,
    cec_tx_answer[tx_answer]);
  hdmi_cec_data.tx_answer = tx_answer;
  wake_up(&tx_cec_queue);
}

void hdmi_delay_ms(unsigned long ms)
{
	u32 timeout = ms*HZ/1000;
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(timeout);
}

static int hdmi_cec_thread(void *parg)
{
  u8 stat;

  printk(KERN_INFO "HDMI hdmi_cec_thread started\n");

  hdmi_cec_configure();
  hdmi_write(HDMI_IH_CEC_STAT0, 0x7f);
  hdmi_write(HDMI_CEC_LOCK, 0x0);

  hdmi_cec_start_receiving();

  while (1) {
    if (kthread_should_stop()) {
      break;
    }

    if (!hdmi_cec_data.cec_state) {
      hdmi_delay_ms(50);
      continue;
    }

    if (hdmi_cec_data.tx_answer == CEC_TX_INPROGRESS) {
      hdmi_cec_start_sending();
      hdmi_cec_send();
    } else {
      hdmi_cec_set_logical_address();
    }

    stat = hdmi_cec_status();
    if (stat & HDMI_IH_CEC_STAT0_ERROR_FOLL) {
      if (hdmi_cec_send_retry() < 0) {
        hdmi_cec_finish_send(CEC_TX_ERROR);
        hdmi_cec_start_receiving();
      }
    } else if (stat & HDMI_IH_CEC_STAT0_NACK) {
      hdmi_cec_finish_send(CEC_TX_NACK);
      hdmi_cec_start_receiving();
    } else if (stat & HDMI_IH_CEC_STAT0_DONE) {
      hdmi_cec_finish_send(CEC_TX_DONE);
      hdmi_cec_start_receiving();
    }
    if (stat & HDMI_IH_CEC_STAT0_EOM) {
      hdmi_delay_ms(10);
      hdmi_cec_receive();
    }

    if (stat == 0) {
      hdmi_delay_ms(10);
    }
  }

  printk(KERN_INFO "HDMI hdmi_cec_thread finished\n");
  return 0;
}

static int __init hdmi_cec_init(void)
{
    int err = 0;
    struct device *temp_class;

    if(!hdmi_base_addr) {
      err = -EBUSY;
      goto out;
    }

    printk(KERN_INFO "HDMI CEC base address: %p\n", hdmi_base_addr);

    hdmi_cec_major = register_chrdev(hdmi_cec_major, "sunxi_hdmi_cec", &hdmi_cec_fops);
    if (hdmi_cec_major < 0) {
        err = -EBUSY;
        goto out;
    }

    hdmi_cec_class = class_create(THIS_MODULE, "sunxi_hdmi_cec");
    if (IS_ERR(hdmi_cec_class)) {
        err = PTR_ERR(hdmi_cec_class);
        goto err_out_chrdev;
    }

    printk(KERN_INFO "HDMI CEC device_create\n");

    temp_class = device_create(hdmi_cec_class, NULL, MKDEV(hdmi_cec_major, 0),
                   NULL, "sunxi_hdmi_cec");
    if (IS_ERR(temp_class)) {
        err = PTR_ERR(temp_class);
        goto err_out_class;
    }

    init_waitqueue_head(&hdmi_cec_queue);
    init_waitqueue_head(&tx_cec_queue);

    INIT_LIST_HEAD(&head);

    mutex_init(&hdmi_cec_data.lock);
    hdmi_cec_data.Logical_address = 15;
    hdmi_cec_data.tx_answer = CEC_TX_AVAIL;
    register_sunxi_hdmi_notifier(&sunxi_hdmi_nb);
    printk(KERN_INFO "HDMI CEC initialized: %s %s\n", __DATE__, __TIME__);
    goto out;

err_out_class:
    device_destroy(hdmi_cec_class, MKDEV(hdmi_cec_major, 0));
    class_destroy(hdmi_cec_class);
err_out_chrdev:
    unregister_chrdev(hdmi_cec_major, "sunxi_hdmi_cec");
out:
    return err;
}

static void __exit hdmi_cec_exit(void)
{
    unregister_sunxi_hdmi_notifier(&sunxi_hdmi_nb);

    if (hdmi_cec_major > 0) {
        device_destroy(hdmi_cec_class, MKDEV(hdmi_cec_major, 0));
        class_destroy(hdmi_cec_class);
        unregister_chrdev(hdmi_cec_major, "sunxi_hdmi_cec");
        hdmi_cec_major = 0;
    }

    return;
}

MODULE_AUTHOR("Joachim Damm");
MODULE_AUTHOR("Kamil Trzciński");
MODULE_DESCRIPTION("Linux HDMI CEC driver for Allwiner H3");
MODULE_LICENSE("GPL");

late_initcall(hdmi_cec_init);
module_exit(hdmi_cec_exit);
