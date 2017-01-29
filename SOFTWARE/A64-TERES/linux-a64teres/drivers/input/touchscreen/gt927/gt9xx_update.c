/* drivers/input/touchscreen/gt813_827_828_update.c
 * 
 * 2010 - 2012 Goodix Technology.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be a reference 
 * to you, when you are integrating the GOODiX's CTP IC into your system, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 * 
 * Version:1.2
 *      V1.0:2012/08/31,first release.
 *      V1.2:2012/10/15,add force update,GT9110P pid map
 */
#include <linux/kthread.h>

#include "gt9xx.h"

#define GUP_REG_HW_INFO             0x4220
#define GUP_REG_FW_MSG              0x41E4
#define GUP_REG_PID_VID             0x8140

#define GUP_SEARCH_FILE_TIMES       5//50
#define UPDATE_FILE_PATH_3          "/system/vendor/firmware/"
#define UPDATE_FILE_PATH_2          "/system/vendor/firmware/goodix_update_gt911.bin"
#define UPDATE_FILE_PATH_1          "/sdcard/goodix/_goodix_update_.bin"

#define FW_HEAD_LENGTH               14
#define FW_SECTION_LENGTH            0x2000
#define FW_DSP_ISP_LENGTH            0x1000
#define FW_DSP_LENGTH                0x1000
#define FW_BOOT_LENGTH               0x800

#define PACK_SIZE                    256
#define MAX_FRAME_CHECK_TIME         5

#define _bRW_MISCTL__SRAM_BANK       0x4048
#define _bRW_MISCTL__MEM_CD_EN       0x4049
#define _bRW_MISCTL__CACHE_EN        0x404B
#define _bRW_MISCTL__TMR0_EN         0x40B0
#define _rRW_MISCTL__SWRST_B0_       0x4180
#define _bWO_MISCTL__CPU_SWRST_PULSE 0x4184
#define _rRW_MISCTL__BOOTCTL_B0_     0x4190
#define _rRW_MISCTL__BOOT_OPT_B0_    0x4218
#define _rRW_MISCTL__BOOT_CTL_       0x5094

#define FAIL    0
#define SUCCESS 1

#pragma pack(1)
typedef struct 
{
    u8  hw_info[4];          //hardware info//
    u8  pid[8];              //product id   //
    u16 vid;                 //version id   //
}st_fw_head;
#pragma pack()

typedef struct
{
    u8 force_update;
    u8 fw_flag;
    struct file *file; 
    st_fw_head  ic_fw_msg;
    mm_segment_t old_fs;
}st_update_msg;

static st_update_msg update_msg;
static struct i2c_client *guitar_client = NULL;
u16 show_len;
u16 total_len;
//extern void int2io(int status,int level);
extern char *firmware_name;

//extern s32  gtp_i2c_read(struct i2c_client *client, uint8_t *buf, s32 len);
//extern s32  gtp_i2c_write(struct i2c_client *client,uint8_t *data,s32 len);
//extern void gtp_reset_guitar(s32 ms);
//extern s32  gtp_send_cfg(struct i2c_client *client);
//extern void gtp_irq_disable(struct goodix_ts_data *ts);
//extern void gtp_irq_enable(struct goodix_ts_data *ts);

static u8 gup_get_ic_msg(struct i2c_client *client, u16 addr, u8* msg, s32 len)
{
    s32 i = 0;

    msg[0] = (addr >> 8) & 0xff;
    msg[1] = addr & 0xff;

    for (i = 0; i < 5; i++)
    {
        if (gtp_i2c_read(client, msg, GTP_ADDR_LENGTH + len) > 0)
        {
            break;
        }
    }

    if (i >= 5)
    {
        GTP_ERROR("Read data from 0x%02x%02x failed!", msg[0], msg[1]);
        return FAIL;
    }

    return SUCCESS;
}

static u8 gup_set_ic_msg(struct i2c_client *client, u16 addr, u8 val)
{
    s32 i = 0;
    u8 msg[3];

    msg[0] = (addr >> 8) & 0xff;
    msg[1] = addr & 0xff;
    msg[2] = val;

    for (i = 0; i < 5; i++)
    {
        if (gtp_i2c_write(client, msg, GTP_ADDR_LENGTH + 1) > 0)
        {
            break;
        }
    }

    if (i >= 5)
    {
        GTP_ERROR("Set data to 0x%02x%02x failed!", msg[0], msg[1]);
        return FAIL;
    }

    return SUCCESS;
}

static u8 gup_get_ic_fw_msg(struct i2c_client *client)
{
    s32 ret = -1;
    u8  retry = 0;
    u8  buf[16];
    u8  i;
    
    //step1:get hardware info
    ret = gup_get_ic_msg(client, GUP_REG_HW_INFO, buf, 4);
    if(FAIL == ret)
    {
        GTP_ERROR("Read hardware info fail.");
        return ret;
    }
    
    memcpy(update_msg.ic_fw_msg.hw_info, &buf[GTP_ADDR_LENGTH], 4);
    for(i=0; i<4; i++)
    {
        update_msg.ic_fw_msg.hw_info[i] = buf[GTP_ADDR_LENGTH + 3 - i];
    } 
    GTP_DEBUG("IC Hardware info:%02x%02x%02x%02x", update_msg.ic_fw_msg.hw_info[0], update_msg.ic_fw_msg.hw_info[1],
                                                   update_msg.ic_fw_msg.hw_info[2], update_msg.ic_fw_msg.hw_info[3]);
    //step2:get firmware message
    for(retry=0; retry<2; retry++)
    {
        ret = gup_get_ic_msg(client, GUP_REG_FW_MSG, buf, 1);
        if(FAIL == ret)
        {
            GTP_ERROR("Read firmware message fail.");
            return ret;
        }
        
        update_msg.force_update = buf[GTP_ADDR_LENGTH];
        if((0xBE != update_msg.force_update)&&(!retry))
        {
            GTP_INFO("The check sum in ic is error.");
            GTP_INFO("The IC will be updated by force.");
            continue;
        }
        break;
    }
    GTP_DEBUG("IC force update flag:0x%x", update_msg.force_update);
    
    //step3:get pid & vid
    ret = gup_get_ic_msg(client, GUP_REG_PID_VID, buf, 6);
    if(FAIL == ret)
    {
        GTP_ERROR("Read product id & version id fail.");
        return ret;
    }
    memset(update_msg.ic_fw_msg.pid, 0, sizeof(update_msg.ic_fw_msg.pid));
    memcpy(update_msg.ic_fw_msg.pid, &buf[GTP_ADDR_LENGTH], 4);
    GTP_DEBUG("IC Product id:%s", update_msg.ic_fw_msg.pid);
    
    //GT9XX PID MAPPING
    /*|-----FLASH-----RAM-----|
      |------918------918-----|
      |------968------968-----|
      |------913------913-----|
      |------913P-----913P----|
      |------927------927-----|
      |------927P-----927P----|
      |------9110-----9110----|
      |------9110P----9111----|*/
    if(update_msg.ic_fw_msg.pid[0] != 0)
    {
        if(!memcmp(update_msg.ic_fw_msg.pid, "9111", 4))
        {
            GTP_DEBUG("IC Mapping Product id:%s", update_msg.ic_fw_msg.pid);
            memcpy(update_msg.ic_fw_msg.pid, "9110P", 5);
        }
    }
    
    update_msg.ic_fw_msg.vid = buf[GTP_ADDR_LENGTH+4] + (buf[GTP_ADDR_LENGTH+5]<<8);
    GTP_DEBUG("IC version id:%04x", update_msg.ic_fw_msg.vid);
    
    return SUCCESS;
}

s32 gup_enter_update_mode(struct i2c_client *client)
{
    s32 ret = -1;
    
    //step1:RST output low last at least 2ms
    //GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
    //gpio_set_one_pin_io_status(gt9xx_reset_hdle, 1, NULL);
    //gpio_write_one_pin_value(gt9xx_reset_hdle,0,NULL);
    gpio_direction_output(config_info.wakeup_gpio.gpio,0);

    msleep(2);
    
    //step2:select I2C slave addr,INT:0--0xBA;1--0x28.
    //GTP_GPIO_OUTPUT(GTP_INT_PORT, (client->addr == 0x14));
    //gpio_set_one_pin_io_status(gt9xx_int_hdle, 1, NULL);
    //gpio_write_one_pin_value(gt9xx_int_hdle,1,NULL);
    //gpio_direction_output(config_info.irq_gpio_number,1);
    int2io(1,1);

    msleep(2);
    
    //step3:RST output high reset guitar
    //GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
    //gpio_write_one_pin_value(gt9xx_reset_hdle,1,NULL);
    gpio_direction_output(config_info.wakeup_gpio.gpio,1);
    msleep(20); 
    //step4:Hold ss51 & dsp
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
    
    return ret;
}

void gup_leave_update_mode(void)
{
//    struct gpio_config_eint_all cfg = {0};
//    //GTP_GPIO_AS_INT(GTP_INT_PORT);
//    //gpio_set_one_pin_io_status(gt9xx_int_hdle, 6, NULL);
//    //__gpio_to_irq(config_info.irq_gpio_number);
//    cfg.gpio 	= CTP_IRQ_NUMBER;
//	cfg.pull 	= GPIO_PULL_DEFAULT;
//	cfg.drvlvl 	= GPIO_DRVLVL_DEFAULT;
//	cfg.enabled	= 0;
//	cfg.trig_type	= CTP_IRQ_MODE;
//	if(0 != sw_gpio_eint_setall_range(&cfg, 1)) {
//        printk("sw_gpio_eint_setall_range failed\n");
//	}
    
    GTP_DEBUG("[leave_update_mode]reset chip.");
    gtp_reset_guitar(20);
}

static u8 gup_enter_upadte_judge(st_fw_head *fw_head)
{
    u16 u16_tmp;
    
    u16_tmp = fw_head->vid;
    fw_head->vid = (u16)(u16_tmp>>8) + (u16)(u16_tmp<<8);

    GTP_DEBUG("FILE HARDWARE INFO:%02x%02x%02x%02x", fw_head->hw_info[0], fw_head->hw_info[1], fw_head->hw_info[2], fw_head->hw_info[3]);
    GTP_DEBUG("FILE PID:%s", fw_head->pid);
    GTP_DEBUG("FILE VID:%04x", fw_head->vid);

    GTP_DEBUG("IC HARDWARE INFO:%02x%02x%02x%02x", update_msg.ic_fw_msg.hw_info[0], update_msg.ic_fw_msg.hw_info[1],
                                                   update_msg.ic_fw_msg.hw_info[2], update_msg.ic_fw_msg.hw_info[3]);
    GTP_DEBUG("IC PID:%s", update_msg.ic_fw_msg.pid);
    GTP_DEBUG("IC VID:%04x", update_msg.ic_fw_msg.vid);

    //First two conditions
    if ( !memcmp(fw_head->hw_info, update_msg.ic_fw_msg.hw_info, sizeof(update_msg.ic_fw_msg.hw_info)))
    {
        GTP_DEBUG("Get the same hardware info.");
        if( update_msg.force_update != 0xBE )
        {
            GTP_INFO("FW chksum error,need enter update.");
            return SUCCESS;
        }
        if (( !memcmp(fw_head->pid, update_msg.ic_fw_msg.pid, strlen(fw_head->pid)))||
            (!memcmp(update_msg.ic_fw_msg.pid, "91XX", 4))||
            (!memcmp(fw_head->pid, "91XX", 4)))
        {
            if(!memcmp(fw_head->pid, "91XX", 4))
            {
                GTP_DEBUG("Force none same pid update mode.");
            }
            else
            {
                GTP_DEBUG("Get the same pid.");
            }
            //The third condition
            if (fw_head->vid > update_msg.ic_fw_msg.vid)
            {

                GTP_INFO("Need enter update.");
                return SUCCESS;
            }
            GTP_ERROR("Don't meet the third condition.");
        }
		else 
			return SUCCESS;
    }    
    return FAIL;
}

static u8 gup_check_update_file(struct i2c_client *client, st_fw_head* fw_head, u8* path)
{
    int ret = 0;
    int i = 0;
    u8 buf[FW_HEAD_LENGTH];

    if (path)
    {
        GTP_DEBUG("Update File path:%s, %d", path, strlen(path));
        update_msg.file = filp_open(path, O_RDONLY, 0644);

        if (IS_ERR(update_msg.file))
        {
            GTP_ERROR("Open update file(%s) error!", path);
            return FAIL;
        }
    }
    else
    {
        //Begin to search update file
        for (i = 0; i < GUP_SEARCH_FILE_TIMES; i++)
        {
            update_msg.file = filp_open(UPDATE_FILE_PATH_1, O_RDONLY, 0444);
            if (IS_ERR(update_msg.file))
            {
                char file_name[128];
                strcpy(file_name,UPDATE_FILE_PATH_3);
                strcat(file_name,firmware_name);
                printk("gt9xx: update file name = %s\n",file_name);
                update_msg.file = filp_open(file_name, O_RDONLY, 0666);
                if (IS_ERR(update_msg.file))
                {
                    GTP_DEBUG("%3d:Searching file...", i);
                    msleep(3000);
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        if (i == GUP_SEARCH_FILE_TIMES)
        {
            GTP_ERROR("Can't find update file.");
            return FAIL;
        }
        GTP_DEBUG("Find the update file.");
    }
    
    update_msg.old_fs = get_fs();
    set_fs(KERNEL_DS);

    update_msg.file->f_op->llseek(update_msg.file, 0, SEEK_SET);
    //update_msg.file->f_pos = 0;

    ret = update_msg.file->f_op->read(update_msg.file, (char*)buf, FW_HEAD_LENGTH, &update_msg.file->f_pos);
    if (ret < 0)
    {
        GTP_ERROR("Read firmware head in update file error.");
        goto load_failed;
    }
    
    //Get the correct nvram data
    //The correct conditions: 
    //1. the hardware info is the same
    //2. the product id is the same
    //3. the firmware version in update file is greater than the firmware version in ic 
    //or the check sum in ic is wrong
    memcpy(fw_head, buf, FW_HEAD_LENGTH);
    
    ret = gup_enter_upadte_judge(fw_head);
    if(SUCCESS == ret)
    {
        GTP_INFO("Check *.bin file success.");
        return SUCCESS;
    }

load_failed:    
    set_fs(update_msg.old_fs);
    filp_close(update_msg.file, NULL);
    return FAIL;
}

#if 0
static u8 gup_check_update_header(struct i2c_client *client, st_fw_head* fw_head)
{
    const u8* pos;
    int i = 0;
    u8 mask_num = 0;
    s32 ret = 0;

    pos = HEADER_UPDATE_DATA;
      
    memcpy(fw_head, pos, FW_HEAD_LENGTH);
    pos += FW_HEAD_LENGTH;

    ret = gup_enter_upadte_judge(fw_head);
    if(SUCCESS == ret)
    {
        return SUCCESS;
    }
    return FAIL;
}
#endif

static u8 gup_burn_proc(struct i2c_client *client, u8 *burn_buf, u16 start_addr, u16 total_length)
{
    s32 ret = 0;
    u16 burn_addr = start_addr;
    u16 frame_length = 0;
    u16 burn_length = 0;
    u8  wr_buf[PACK_SIZE + GTP_ADDR_LENGTH];
    u8  rd_buf[PACK_SIZE + GTP_ADDR_LENGTH];
    u8  retry = 0;
    
    GTP_DEBUG("Begin burn %dk data to addr 0x%x", (total_length/1024), start_addr);
    while(burn_length < total_length)
    {
        GTP_DEBUG("B/T:%04d/%04d", burn_length, total_length);
        frame_length = ((total_length - burn_length) > PACK_SIZE) ? PACK_SIZE : (total_length - burn_length);
        wr_buf[0] = (u8)(burn_addr>>8);
        rd_buf[0] = wr_buf[0];
        wr_buf[1] = (u8)burn_addr;
        rd_buf[1] = wr_buf[1];
        memcpy(&wr_buf[GTP_ADDR_LENGTH], &burn_buf[burn_length], frame_length);
        
        for(retry = 0; retry < MAX_FRAME_CHECK_TIME; retry++)
        {
            ret = gtp_i2c_write(client, wr_buf, GTP_ADDR_LENGTH + frame_length);
            if(ret <= 0)
            {
                GTP_ERROR("Write frame data i2c error.");
                continue;
            }
            ret = gtp_i2c_read(client, rd_buf, GTP_ADDR_LENGTH + frame_length);
            if(ret <= 0)
            {
                GTP_ERROR("Read back frame data i2c error.");
                continue;
            }
            
            if(memcmp(&wr_buf[GTP_ADDR_LENGTH], &rd_buf[GTP_ADDR_LENGTH], frame_length))
            {
                GTP_ERROR("Check frame data fail,not equal.");
                GTP_DEBUG("write array:");
                GTP_DEBUG_ARRAY(&wr_buf[GTP_ADDR_LENGTH], frame_length);
                GTP_DEBUG("read array:");
                GTP_DEBUG_ARRAY(&rd_buf[GTP_ADDR_LENGTH], frame_length);
                continue;
            }
            else
            {
                //GTP_DEBUG("Check frame data success.");
                break;
            }
        }
        if(retry >= MAX_FRAME_CHECK_TIME)
        {
            GTP_ERROR("Burn frame data time out,exit.");
            return FAIL;
        }
        burn_length += frame_length;
        burn_addr += frame_length;
    }
    return SUCCESS;
}

static u8 gup_load_section_file(u8* buf, u16 offset, u16 length)
{
    s32 ret = 0;
    
    if(update_msg.file == NULL)
    {
        GTP_ERROR("cannot find update file,load section file fail.");
        return FAIL;
    }
    update_msg.file->f_pos = FW_HEAD_LENGTH + offset;
    
    ret = update_msg.file->f_op->read(update_msg.file, (char*)buf, length, &update_msg.file->f_pos);
    if(ret < 0)
    {
        GTP_ERROR("Read update file fail.");
        return FAIL;
    }
    
    return SUCCESS;
}

static u8 gup_recall_check(struct i2c_client *client, u8* chk_src, u16 start_rd_addr, u16 chk_length)
{
    u8  rd_buf[PACK_SIZE + GTP_ADDR_LENGTH];
    s32 ret = 0;
    u16 recall_addr = start_rd_addr;
    u16 recall_length = 0;
    u16 frame_length = 0;

    while(recall_length < chk_length)
    {
        frame_length = ((chk_length - recall_length) > PACK_SIZE) ? PACK_SIZE : (chk_length - recall_length);
        ret = gup_get_ic_msg(client, recall_addr, rd_buf, frame_length);
        if(ret <= 0)
        {
            GTP_ERROR("recall i2c error,exit");
            return FAIL;
        }
        
        if(memcmp(&rd_buf[GTP_ADDR_LENGTH], &chk_src[recall_length], frame_length))
        {
            GTP_ERROR("Recall frame data fail,not equal.");
            GTP_DEBUG("chk_src array:");
            GTP_DEBUG_ARRAY(&chk_src[recall_length], frame_length);
            GTP_DEBUG("recall array:");
            GTP_DEBUG_ARRAY(&rd_buf[GTP_ADDR_LENGTH], frame_length);
            return FAIL;
        }
        
        recall_length += frame_length;
        recall_addr += frame_length;
    }
    GTP_DEBUG("Recall check %dk firmware success.", (chk_length/1024));
    
    return SUCCESS;
}

static u8 gup_burn_fw_section(struct i2c_client *client, u8 *fw_section, u16 start_addr, u8 bank_cmd )
{
    s32 ret = 0;
    u8  rd_buf[5];
  
    //step1:hold ss51 & dsp
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]hold ss51 & dsp fail.");
        return FAIL;
    }
    
    //step2:set scramble
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_OPT_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]set scramble fail.");
        return FAIL;
    }
    
    //step3:select bank
    ret = gup_set_ic_msg(client, _bRW_MISCTL__SRAM_BANK, (bank_cmd >> 4)&0x0F);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]select bank %d fail.", (bank_cmd >> 4)&0x0F);
        return FAIL;
    }
    
    //step4:enable accessing code
    ret = gup_set_ic_msg(client, _bRW_MISCTL__MEM_CD_EN, 0x01);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]enable accessing code fail.");
        return FAIL;
    }
    
    //step5:burn 8k fw section
    ret = gup_burn_proc(client, fw_section, start_addr, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_section]burn fw_section fail.");
        return FAIL;
    }
    
    //step6:hold ss51 & release dsp
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x04);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]hold ss51 & release dsp fail.");
        return FAIL;
    }
    //must delay
    msleep(1);
    
    //step7:send burn cmd to move data to flash from sram
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, bank_cmd&0x0f);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]send burn cmd fail.");
        return FAIL;
    }
    GTP_DEBUG("[burn_fw_section]Wait for the burn is complete......");
    do{
        ret = gup_get_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, rd_buf, 1);
        if(ret <= 0)
        {
            GTP_ERROR("[burn_fw_section]Get burn state fail");
            return FAIL;
        }
        msleep(10);
        //GTP_DEBUG("[burn_fw_section]Get burn state:%d.", rd_buf[GTP_ADDR_LENGTH]);
    }while(rd_buf[GTP_ADDR_LENGTH]);

    //step8:select bank
    ret = gup_set_ic_msg(client, _bRW_MISCTL__SRAM_BANK, (bank_cmd >> 4)&0x0F);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]select bank %d fail.", (bank_cmd >> 4)&0x0F);
        return FAIL;
    }
    
    //step9:enable accessing code
    ret = gup_set_ic_msg(client, _bRW_MISCTL__MEM_CD_EN, 0x01);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]enable accessing code fail.");
        return FAIL;
    }
    
    //step10:recall 8k fw section
    ret = gup_recall_check(client, fw_section, start_addr, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_section]recall check 8k firmware fail.");
        return FAIL;
    }
    
    //step11:disable accessing code
    ret = gup_set_ic_msg(client, _bRW_MISCTL__MEM_CD_EN, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_section]disable accessing code fail.");
        return FAIL;
    }
    
    return SUCCESS;
}

static u8 gup_burn_dsp_isp(struct i2c_client *client)
{
    s32 ret = 0;
    u8* fw_dsp_isp = NULL;
    u8  retry = 0;
    
    GTP_DEBUG("[burn_dsp_isp]Begin burn dsp isp---->>");
    
    //step1:alloc memory
    GTP_DEBUG("[burn_dsp_isp]step1:alloc memory");
    while(retry++ < 5)
    {
        fw_dsp_isp = (u8*)kzalloc(FW_DSP_ISP_LENGTH, GFP_KERNEL);
        if(fw_dsp_isp == NULL)
        {
            continue;
        }
        else
        {
            GTP_INFO("[burn_dsp_isp]Alloc %dk byte memory success.", (FW_DSP_ISP_LENGTH/1024));
            break;
        }
    }
    if(retry >= 5)
    {
        GTP_ERROR("[burn_dsp_isp]Alloc memory fail,exit.");
	if(NULL != fw_dsp_isp)
		kfree(fw_dsp_isp);
        return FAIL;
    }
    
    //step2:load dsp isp file data
    GTP_DEBUG("[burn_dsp_isp]step2:load dsp isp file data");
    ret = gup_load_section_file(fw_dsp_isp, (4*FW_SECTION_LENGTH+FW_DSP_LENGTH+FW_BOOT_LENGTH), FW_DSP_ISP_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_dsp_isp]load firmware dsp_isp fail.");
        goto exit_burn_dsp_isp;
    }
    
    //step3:disable wdt,clear cache enable
    GTP_DEBUG("[burn_dsp_isp]step3:disable wdt,clear cache enable");
    ret = gup_set_ic_msg(client, _bRW_MISCTL__TMR0_EN, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]disable wdt fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    ret = gup_set_ic_msg(client, _bRW_MISCTL__CACHE_EN, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]clear cache enable fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step4:hold ss51 & dsp
    GTP_DEBUG("[burn_dsp_isp]step4:hold ss51 & dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]hold ss51 & dsp fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step5:set boot from sram
    GTP_DEBUG("[burn_dsp_isp]step5:set boot from sram");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOTCTL_B0_, 0x02);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]set boot from sram fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step6:software reboot
    GTP_DEBUG("[burn_dsp_isp]step6:software reboot");
    ret = gup_set_ic_msg(client, _bWO_MISCTL__CPU_SWRST_PULSE, 0x01);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]software reboot fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step7:select bank2
    GTP_DEBUG("[burn_dsp_isp]step7:select bank2");
    ret = gup_set_ic_msg(client, _bRW_MISCTL__SRAM_BANK, 0x02);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]select bank2 fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step8:enable accessing code
    GTP_DEBUG("[burn_dsp_isp]step8:enable accessing code");
    ret = gup_set_ic_msg(client, _bRW_MISCTL__MEM_CD_EN, 0x01);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]enable accessing code fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    
    //step9:burn 4k dsp_isp
    GTP_DEBUG("[burn_dsp_isp]step9:burn 4k dsp_isp");
    ret = gup_burn_proc(client, fw_dsp_isp, 0xC000, FW_DSP_ISP_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_dsp_isp]burn dsp_isp fail.");
        goto exit_burn_dsp_isp;
    }
    
    //step10:set scramble
    GTP_DEBUG("[burn_dsp_isp]step10:set scramble");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_OPT_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_dsp_isp]set scramble fail.");
        ret = FAIL;
        goto exit_burn_dsp_isp;
    }
    ret = SUCCESS;

exit_burn_dsp_isp:
    kfree(fw_dsp_isp);
    return ret;
}

static u8 gup_burn_fw_ss51(struct i2c_client *client)
{
    u8* fw_ss51 = NULL;
    u8  retry = 0;
    s32 ret = 0;
    
    GTP_DEBUG("[burn_fw_ss51]Begin burn ss51 firmware---->>");
    
    //step1:alloc memory
    GTP_DEBUG("[burn_fw_ss51]step1:alloc memory");
    while(retry++ < 5)
    {
        fw_ss51 = (u8*)kzalloc(FW_SECTION_LENGTH, GFP_KERNEL);
        if(fw_ss51 == NULL)
        {
            continue;
        }
        else
        {
            GTP_INFO("[burn_fw_ss51]Alloc %dk byte memory success.", (FW_SECTION_LENGTH/1024));
            break;
        }
    }
    if(retry >= 5)
    {
	if(NULL != fw_ss51)
		kfree(fw_ss51);
	GTP_ERROR("[burn_fw_ss51]Alloc memory fail,exit.");
        return FAIL;
    }
    
    //step2:load ss51 firmware section 1 file data
    GTP_DEBUG("[burn_fw_ss51]step2:load ss51 firmware section 1 file data");
    ret = gup_load_section_file(fw_ss51, 0, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]load ss51 firmware section 1 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step3:clear control flag
    GTP_DEBUG("[burn_fw_ss51]step3:clear control flag");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_ss51]clear control flag fail.");
        ret = FAIL;
        goto exit_burn_fw_ss51;
    }
    
    //step4:burn ss51 firmware section 1
    GTP_DEBUG("[burn_fw_ss51]step4:burn ss51 firmware section 1");
    ret = gup_burn_fw_section(client, fw_ss51, 0xC000, 0x01);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]burn ss51 firmware section 1 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step5:load ss51 firmware section 2 file data
    GTP_DEBUG("[burn_fw_ss51]step5:load ss51 firmware section 2 file data");
    ret = gup_load_section_file(fw_ss51, FW_SECTION_LENGTH, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]load ss51 firmware section 2 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step6:burn ss51 firmware section 2
    GTP_DEBUG("[burn_fw_ss51]step6:burn ss51 firmware section 2");
    ret = gup_burn_fw_section(client, fw_ss51, 0xE000, 0x02);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]burn ss51 firmware section 2 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step7:load ss51 firmware section 3 file data
    GTP_DEBUG("[burn_fw_ss51]step7:load ss51 firmware section 3 file data");
    ret = gup_load_section_file(fw_ss51, 2*FW_SECTION_LENGTH, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]load ss51 firmware section 3 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step8:burn ss51 firmware section 3
    GTP_DEBUG("[burn_fw_ss51]step8:burn ss51 firmware section 3");
    ret = gup_burn_fw_section(client, fw_ss51, 0xC000, 0x13);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]burn ss51 firmware section 3 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step9:load ss51 firmware section 4 file data
    GTP_DEBUG("[burn_fw_ss51]step9:load ss51 firmware section 4 file data");
    ret = gup_load_section_file(fw_ss51, 3*FW_SECTION_LENGTH, FW_SECTION_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]load ss51 firmware section 4 fail.");
        goto exit_burn_fw_ss51;
    }
    
    //step10:burn ss51 firmware section 4
    GTP_DEBUG("[burn_fw_ss51]step10:burn ss51 firmware section 4");
    ret = gup_burn_fw_section(client, fw_ss51, 0xE000, 0x14);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_ss51]burn ss51 firmware section 4 fail.");
        goto exit_burn_fw_ss51;
    }
    
    ret = SUCCESS;
    
exit_burn_fw_ss51:
    kfree(fw_ss51);
    return ret;
}

static u8 gup_burn_fw_dsp(struct i2c_client *client)
{
    s32 ret = 0;
    u8* fw_dsp = NULL;
    u8  retry = 0;
    u8  rd_buf[5];
    
    GTP_DEBUG("[burn_fw_dsp]Begin burn dsp firmware---->>");
    //step1:alloc memory
    GTP_DEBUG("[burn_fw_dsp]step1:alloc memory");
    while(retry++ < 5)
    {
        fw_dsp = (u8*)kzalloc(FW_DSP_LENGTH, GFP_KERNEL);
        if(fw_dsp == NULL)
        {
            continue;
        }
        else
        {
            GTP_INFO("[burn_fw_dsp]Alloc %dk byte memory success.", (FW_SECTION_LENGTH/1024));
            break;
        }
    }
    if(retry >= 5)
    {
        GTP_ERROR("[burn_fw_dsp]Alloc memory fail,exit.");
	if(NULL != fw_dsp) 
		kfree(fw_dsp);
        return FAIL;
    }
    
    //step2:load firmware dsp
    GTP_DEBUG("[burn_fw_dsp]step2:load firmware dsp");
    ret = gup_load_section_file(fw_dsp, 4*FW_SECTION_LENGTH, FW_DSP_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_dsp]load firmware dsp fail.");
        goto exit_burn_fw_dsp;
    }
    
    //step3:select bank3
    GTP_DEBUG("[burn_fw_dsp]step3:select bank3");
    ret = gup_set_ic_msg(client, _bRW_MISCTL__SRAM_BANK, 0x03);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_dsp]select bank3 fail.");
        ret = FAIL;
        goto exit_burn_fw_dsp;
    }
    
    //step4:hold ss51 & dsp
    GTP_DEBUG("[burn_fw_dsp]step4:hold ss51 & dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_dsp]hold ss51 & dsp fail.");
        ret = FAIL;
        goto exit_burn_fw_dsp;
    }
    
    //step5:set scramble
    GTP_DEBUG("[burn_fw_dsp]step5:set scramble");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_OPT_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_dsp]set scramble fail.");
        ret = FAIL;
        goto exit_burn_fw_dsp;
    }
    
    //step6:release ss51 & dsp
    GTP_DEBUG("[burn_fw_dsp]step6:release ss51 & dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_dsp]release ss51 & dsp fail.");
        ret = FAIL;
        goto exit_burn_fw_dsp;
    }
    //must delay
    msleep(1);
    
    //step7:burn 4k dsp firmware
    GTP_DEBUG("[burn_fw_dsp]step7:burn 4k dsp firmware");
    ret = gup_burn_proc(client, fw_dsp, 0x9000, FW_DSP_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_dsp]burn fw_section fail.");
        goto exit_burn_fw_dsp;
    }
    
    //step8:send burn cmd to move data to flash from sram
    GTP_DEBUG("[burn_fw_dsp]step8:send burn cmd to move data to flash from sram");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, 0x05);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_dsp]send burn cmd fail.");
        goto exit_burn_fw_dsp;
    }
    GTP_DEBUG("[burn_fw_dsp]Wait for the burn is complete......");
    do{
        ret = gup_get_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, rd_buf, 1);
        if(ret <= 0)
        {
            GTP_ERROR("[burn_fw_dsp]Get burn state fail");
            goto exit_burn_fw_dsp;
        }
        msleep(10);
        //GTP_DEBUG("[burn_fw_dsp]Get burn state:%d.", rd_buf[GTP_ADDR_LENGTH]);
    }while(rd_buf[GTP_ADDR_LENGTH]);
    
    //step9:recall check 4k dsp firmware
    GTP_DEBUG("[burn_fw_dsp]step9:recall check 4k dsp firmware");
    ret = gup_recall_check(client, fw_dsp, 0x9000, FW_DSP_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_dsp]recall check 4k dsp firmware fail.");
        goto exit_burn_fw_dsp;
    }
    
    ret = SUCCESS;
    
exit_burn_fw_dsp:
    kfree(fw_dsp);
    return ret;
}

static u8 gup_burn_fw_boot(struct i2c_client *client)
{
    s32 ret = 0;
    u8* fw_boot = NULL;
    u8  retry = 0;
    u8  rd_buf[5];
    
    GTP_DEBUG("[burn_fw_boot]Begin burn bootloader firmware---->>");
    
    //step1:Alloc memory
    GTP_DEBUG("[burn_fw_boot]step1:Alloc memory");
    while(retry++ < 5)
    {
        fw_boot = (u8*)kzalloc(FW_BOOT_LENGTH, GFP_KERNEL);
        if(fw_boot == NULL)
        {
            continue;
        }
        else
        {
            GTP_INFO("[burn_fw_boot]Alloc %dk byte memory success.", (FW_BOOT_LENGTH/1024));
            break;
        }
    }
    if(retry >= 5)
    {
        GTP_ERROR("[burn_fw_boot]Alloc memory fail,exit.");
	if( NULL != fw_boot)
		kfree(fw_boot);
        return FAIL;
    }
    
    //step2:load firmware bootloader
    GTP_DEBUG("[burn_fw_boot]step2:load firmware bootloader");
    ret = gup_load_section_file(fw_boot, (4*FW_SECTION_LENGTH+FW_DSP_LENGTH), FW_BOOT_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_boot]load firmware dsp fail.");
        goto exit_burn_fw_boot;
    }
    
    //step3:hold ss51 & dsp
    GTP_DEBUG("[burn_fw_boot]step3:hold ss51 & dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x0C);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]hold ss51 & dsp fail.");
        ret = FAIL;
        goto exit_burn_fw_boot;
    }
    
    //step4:set scramble
    GTP_DEBUG("[burn_fw_boot]step4:set scramble");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_OPT_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]set scramble fail.");
        ret = FAIL;
        goto exit_burn_fw_boot;
    }
    
    //step5:release ss51 & dsp
    GTP_DEBUG("[burn_fw_boot]step5:release ss51 & dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x00);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]release ss51 & dsp fail.");
        ret = FAIL;
        goto exit_burn_fw_boot;
    }
    //must delay
    msleep(1);
    
    //step6:burn 2k bootloader firmware
    GTP_DEBUG("[burn_fw_boot]step6:burn 2k bootloader firmware");
    ret = gup_burn_proc(client, fw_boot, 0x9000, FW_BOOT_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_boot]burn fw_section fail.");
        goto exit_burn_fw_boot;
    }
    
    //step7:send burn cmd to move data to flash from sram
    GTP_DEBUG("[burn_fw_boot]step7:send burn cmd to move data to flash from sram");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, 0x06);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]send burn cmd fail.");
        goto exit_burn_fw_boot;
    }
    GTP_DEBUG("[burn_fw_boot]Wait for the burn is complete......");
    do{
        ret = gup_get_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, rd_buf, 1);
        if(ret <= 0)
        {
            GTP_ERROR("[burn_fw_boot]Get burn state fail");
            goto exit_burn_fw_boot;
        }
        msleep(10);
        //GTP_DEBUG("[burn_fw_boot]Get burn state:%d.", rd_buf[GTP_ADDR_LENGTH]);
    }while(rd_buf[GTP_ADDR_LENGTH]);
    
    //step8:recall check 2k bootloader firmware
    GTP_DEBUG("[burn_fw_boot]step8:recall check 2k bootloader firmware");
    ret = gup_recall_check(client, fw_boot, 0x9000, FW_BOOT_LENGTH);
    if(FAIL == ret)
    {
        GTP_ERROR("[burn_fw_boot]recall check 4k dsp firmware fail.");
        goto exit_burn_fw_boot;
    }
    
    //step9:enable download DSP code 
    GTP_DEBUG("[burn_fw_boot]step9:enable download DSP code ");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__BOOT_CTL_, 0x99);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]enable download DSP code fail.");
        ret = FAIL;
        goto exit_burn_fw_boot;
    }
    
    //step10:release ss51 & hold dsp
    GTP_DEBUG("[burn_fw_boot]step10:release ss51 & hold dsp");
    ret = gup_set_ic_msg(client, _rRW_MISCTL__SWRST_B0_, 0x08);
    if(ret <= 0)
    {
        GTP_ERROR("[burn_fw_boot]release ss51 & hold dsp fail.");
        ret = FAIL;
        goto exit_burn_fw_boot;
    }
    
    ret = SUCCESS;
    
exit_burn_fw_boot:
    kfree(fw_boot);
    return ret;
}

s32 gup_update_proc(void *dir)
{
    s32 ret = 0;
    u8  retry = 0;
    st_fw_head fw_head;
    struct goodix_ts_data *ts = NULL;
    
    GTP_DEBUG("[update_proc]Begin update ......");
    
    ts = i2c_get_clientdata(guitar_client);
    show_len = 1;
    total_len = 100;
    if(dir == NULL)
    {
        msleep(3000);                               //wait main thread to be completed
    }
    gtp_reset_guitar(20);
    ret = gup_get_ic_fw_msg(guitar_client);
    if(FAIL == ret)
    {
        GTP_ERROR("[update_proc]get ic message fail.");
        goto update_fail;
    }    

    ret = gup_check_update_file(guitar_client, &fw_head, (u8*)dir);
    if(FAIL == ret)
    {
        GTP_ERROR("[update_proc]check update file fail.");
        goto update_fail;
    }
   	
   	//stone 20140103
    //gtp_irq_disable(ts);
	//sw_gpio_eint_set_enable(CTP_IRQ_NUMBER,0);
//#if GTP_ESD_PROTECT
//    ts->gtp_is_suspend = 1;
//#endif

    ret = gup_enter_update_mode(guitar_client);
    if(FAIL == ret)
    {
         GTP_ERROR("[update_proc]enter update mode fail.");
         goto update_fail;
    }
    
    while(retry++ < 5)
    {
        show_len = 10;
        total_len = 100;
        ret = gup_burn_dsp_isp(guitar_client);
        if(FAIL == ret)
        {
            GTP_ERROR("[update_proc]burn dsp isp fail.");
            continue;
        }
        
        show_len += 10;
        ret = gup_burn_fw_ss51(guitar_client);
        if(FAIL == ret)
        {
            GTP_ERROR("[update_proc]burn ss51 firmware fail.");
            continue;
        }
        
        show_len += 40;
        ret = gup_burn_fw_dsp(guitar_client);
        if(FAIL == ret)
        {
            GTP_ERROR("[update_proc]burn dsp firmware fail.");
            continue;
        }
        
        show_len += 20;
        ret = gup_burn_fw_boot(guitar_client);
        if(FAIL == ret)
        {
            GTP_ERROR("[update_proc]burn bootloader firmware fail.");
            continue;
        }
        show_len += 10;
        GTP_INFO("[update_proc]UPDATE SUCCESS.");
        break;
    }
    if(retry >= 5)
    {
        GTP_ERROR("[update_proc]retry timeout,UPDATE FAIL.");
        goto update_fail;
    }
    
    GTP_DEBUG("[update_proc]leave update mode.");
    gup_leave_update_mode();
    
    
    msleep(100);
    GTP_DEBUG("[update_proc]send config.");
    ret = gtp_send_cfg(guitar_client);
    if(ret < 0)
    {
        GTP_ERROR("[update_proc]send config fail.");
    }
    
    //stone 20140103
    //gtp_irq_enable(ts);
	//sw_gpio_eint_set_enable(CTP_IRQ_NUMBER,0);
//#if GTP_ESD_PROTECT
//    ts->gtp_is_suspend = 0;
//#endif

    show_len = 100;
    total_len = 100;
    //sw_gpio_eint_set_enable(CTP_IRQ_NUMBER,1);
	gtp_irq_enable(ts);
    return SUCCESS;
update_fail:
    show_len = 200;
    total_len = 100;
    gup_leave_update_mode();
    //sw_gpio_eint_set_enable(CTP_IRQ_NUMBER,1);
	gtp_irq_enable(ts); 
    return FAIL;
}

u8 gup_init_update_proc(struct goodix_ts_data *ts)
{
    struct task_struct *thread = NULL;

    GTP_INFO("Ready to run update thread.");

    guitar_client = ts->client;

    thread = kthread_run(gup_update_proc, (void*)NULL, "guitar_update");
    if (IS_ERR(thread))
    {
        GTP_ERROR("Failed to create update thread.\n");
        return -1;
    }

    return 0;
}
