/*
Copyright (c) 2012-2022 ChipOne Technology (Beijing) Co., Ltd. All Rights Reserved.
This PROPRIETARY SOFTWARE is the property of ChipOne Technology (Beijing) Co., Ltd.
and may contains trade secrets and/or other confidential information of ChipOne
Technology (Beijing) Co., Ltd. This file shall not be disclosed to any third party,
in whole or in part, without prior written consent of ChipOne.
THIS PROPRIETARY SOFTWARE & ANY RELATED DOCUMENTATION ARE PROVIDED AS IS,
WITH ALL FAULTS, & WITHOUT WARRANTY OF ANY KIND. CHIPONE DISCLAIMS ALL EXPRESS OR
IMPLIED WARRANTIES.

File Name:    flash.c
Abstract:
              flash operation, read write etc.
Author:       Zhimin Tian
Date :        10 30,2012
Version:      0.1[.revision]
History :
Change logs.
*/
#include "icn83xx.h"

static struct ctp_config_info config_info = {
	    .input_type = CTP_TYPE,
	    .name = NULL,
};

extern int ctp_wakeup(int status,int ms);


struct file  *fp;
int g_status = R_OK;
static char fw_mode = 0;
static int fw_size = 0;
static unsigned char *fw_buf;

void icn83xx_rawdatadump(short *mem, int size, char br)
{
        int i;
        for(i=0;i<size; i++) {
                if((i!=0)&&(i%br == 0))
                        printk("\n");
                printk(" %5d", mem[i]);
        }
        printk("\n");
}

void icn83xx_memdump(char *mem, int size)
{
        int i;
        for(i=0;i<size; i++) {
                if(i%16 == 0)
                        printk("\n");
                printk(" 0x%2x", mem[i]);
        }
        printk("\n");
}

int  icn83xx_checksum(int sum, char *buf, unsigned int size)
{
        int i;
        for(i=0; i<size; i++) {
                sum = sum + buf[i];
        }
        return sum;
}


int icn83xx_update_status(int status)
{
        //flash_info("icn83xx_update_status: %d\n", status);
        g_status = status;
        return 0;
}

int icn83xx_get_status(void)
{
        return  g_status;
}

void icn83xx_set_fw(int size, unsigned char *buf)
{
        fw_size = size;
        fw_buf = buf;
}

/***********************************************************************************************
Name    :   icn83xx_writeInfo
Input   :   addr, value
Output  :
function    :   write Flash Info
***********************************************************************************************/

int icn83xx_writeInfo(unsigned short addr, char value)
{
        int ret = -1;
        char temp_buf[3];

        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(230, temp_buf, 2);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);
        temp_buf[0] = value;
        ret = icn83xx_i2c_txdata(232, temp_buf, 1);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(5);
        return 0;
}
/***********************************************************************************************
Name    :   icn83xx_readInfo
Input   :
Output  :
function    :   read Flash info
***********************************************************************************************/

int icn83xx_readInfo(unsigned short addr, char *value)
{
        int ret = -1;
        char temp_buf[3];

        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(230, temp_buf, 2);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);
        ret = icn83xx_i2c_rxdata(232, value, 1);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);
        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_writeReg
Input   :   addr, value
Output  :
function    :   write MCU xdata and reg
***********************************************************************************************/

int icn83xx_writeReg(unsigned short addr, char value)
{
        int ret = -1;
        char temp_buf[3];

        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(224, temp_buf, 2);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);
        temp_buf[0] = value;
        ret = icn83xx_i2c_txdata(226, temp_buf, 1);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(5);
        return 0;
}
/***********************************************************************************************
Name    :   icn83xx_readReg
Input   :
Output  :
function    :   read MCU xdata and reg
***********************************************************************************************/

int icn83xx_readReg(unsigned short addr, char *value)
{
        int ret = -1;
        char temp_buf[3];

        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(224, temp_buf, 2);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);

        ret = icn83xx_i2c_rxdata(226, value, 1);
        if (ret < 0) {
                op_error("%s failed! ret: %d\n", __func__, ret);
                return -1;
        }
        mdelay(2);
        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_open_fw
Input   :   *fw

Output  :   file size
function    :   open the fw file, and return total size
***********************************************************************************************/
int  icn83xx_open_fw( char *fw)
{
        int file_size;
        mm_segment_t fs;
        struct inode *inode = NULL;
        if(strcmp(fw, "icn83xx_firmware") == 0) {
                fw_mode = 1;  //use inner array
                return fw_size;
        } else {
                fw_mode = 0; //use file in file system
        }

        fp = filp_open(fw, O_RDONLY, 0);
        if (IS_ERR(fp)) {
                flash_error("read fw file error\n");
                return -1;
        }
        else
                flash_info("open fw file ok\n");

        inode = fp->f_dentry->d_inode;
        file_size = inode->i_size;
        flash_info("file size: %d\n", file_size);

        fs = get_fs();
        set_fs(KERNEL_DS);

        return  file_size;

}

/***********************************************************************************************
Name    :   icn83xx_read_fw
Input   :   offset
            length, read length
            buf, return buffer
Output  :
function    :   read data to buffer
***********************************************************************************************/
int  icn83xx_read_fw(int offset, int length, char *buf)
{
        loff_t  pos = offset;
        if(fw_mode == 1) {
                memcpy(buf, fw_buf+offset, length);
        } else {
                vfs_read(fp, buf, length, &pos);
        }
        //  icn83xx_memdump(buf, length);
        return 0;
}


/***********************************************************************************************
Name    :   icn83xx_close_fw
Input   :
Output  :
function    :   close file
***********************************************************************************************/
int  icn83xx_close_fw(void)
{
        if(fw_mode == 0) {
                filp_close(fp, NULL);
        }

        return 0;
}
/***********************************************************************************************
Name    :   icn83xx_readVersion
Input   :   void
Output  :
function    :   return version
***********************************************************************************************/
int icn83xx_readVersion(void)
{
        int err = 0;
        char tmp[2];
        short CurVersion;
        err = icn83xx_i2c_rxdata(12, tmp, 2);
        if (err < 0) {
                calib_error("%s failed: %d\n", __func__, err);
                return err;
        }
        CurVersion = (tmp[0]<<8) | tmp[1];
        return CurVersion;
}

/***********************************************************************************************
Name    :   icn83xx_changemode
Input   :   normal/factory/config
Output  :
function    :   change work mode
***********************************************************************************************/
int icn83xx_changemode(char mode)
{
        char value = 0x0;
        icn83xx_write_reg(0, mode);
        mdelay(1);
        icn83xx_read_reg(1, &value);
        while(value != 0) {
                mdelay(1);
                icn83xx_read_reg(1, &value);
        }
        //calib_info("icn83xx_changemode ok\n");
        return 0;
}


/***********************************************************************************************
Name    :   icn83xx_readrawdata
Input   :   rownum and length
Output  :
function    :   read one row rawdata
***********************************************************************************************/

int icn83xx_readrawdata(char *buffer, char row, char length)
{
        int err = 0;
        int i;
       // calib_info("readrawdata: %d, length: %d\n", row, length);
        icn83xx_write_reg(3, row);
        mdelay(1);
        err = icn83xx_i2c_rxdata(160, buffer, length);
        if (err < 0) {
                calib_error("%s failed: %d\n", __func__, err);
                return err;
        }

        for(i=0; i<length; i=i+2) {
                swap_ab(buffer[i], buffer[i+1]);
        }
        return err;
}

/***********************************************************************************************
Name    :   icn83xx_scanTP
Input   :
Output  :
function    :   scan one frame rawdata
***********************************************************************************************/

int icn83xx_scanTP(void)
{
        char value = 0;
        icn83xx_write_reg(2, 0x0);
        mdelay(1);
        icn83xx_read_reg(2, &value);
        while(value != 1) {
                mdelay(1);
                icn83xx_read_reg(2, &value);
        }
       // calib_info("icn83xx_scanTP ok\n");
        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_readTP
Input   :   rownum and columnnum
Output  :
function    :   read one frame rawdata
***********************************************************************************************/

int icn83xx_readTP(char row_num, char column_num, char *buffer)
{
        int err = 0;
        int i;
        //calib_info("icn83xx_readTP\n");
        icn83xx_changemode(1);
        icn83xx_scanTP();
        for(i=0; i<row_num; i++) {
                icn83xx_readrawdata(&buffer[i*16*2], i, column_num*2);
        }
        icn83xx_changemode(0);
        return err;
}


/***********************************************************************************************
Name    :   icn83xx_goto_progmode
Input   :
Output  :
function    :   change MCU to progmod
***********************************************************************************************/
int icn83xx_goto_progmode(void)
{
        int ret = -1;
        char regValue = 0;

        flash_info("icn83xx_goto_progmode\n");

        ret = icn83xx_readReg(0x009, &regValue);
        if(ret != 0)
                return ret;
        flash_info("[0x009]: 0x%x\n", regValue);

        //open clock
        if(regValue != 0xDF) {
                icn83xx_changemode(2);
                ret = icn83xx_writeReg(0x002, 0x00);
                if(ret != 0)
                        return ret;
                ret = icn83xx_writeReg(0x009, 0xDF);
                if(ret != 0)
                        return ret;
                ret = icn83xx_writeReg(0x010, 0x00);
                if(ret != 0)
                        return ret;

        }

/*
        addr = 0x0;
        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(230, temp_buf, 2);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }

        temp_buf[0] = 0xff;
        ret = icn83xx_i2c_txdata(232, temp_buf, 1);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }
*/
        ret = icn83xx_writeInfo(0x0, 0xff);
        if(ret != 0)
                return ret;

/*
        addr = 0x1;
        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(230, temp_buf, 2);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }

        temp_buf[0] = 0xff;
        ret = icn83xx_i2c_txdata(232, temp_buf, 1);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }
*/
        ret = icn83xx_writeInfo(0x1, 0xff);
        if(ret != 0)
                return ret;

        ret = icn83xx_writeInfo(0x10, 0xff);
        if(ret != 0)
                return ret;

        ret = icn83xx_writeInfo(0x11, 0xff);
        if(ret != 0)
                return ret;
/*
        addr = 0xf00;
        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        ret = icn83xx_i2c_txdata(224, temp_buf, 2);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }
        temp_buf[0] = 0x1;
        ret = icn83xx_i2c_txdata(226, temp_buf, 1);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }
*/
        ret = icn83xx_writeReg(0xf00, 1);
        if(ret != 0)
                return ret;
        input_fetch_sysconfig_para(&(config_info.input_type));
        ctp_wakeup(0, CTP_RESET_LOW_PERIOD);
        msleep(CTP_RESET_HIGH_PERIOD);
        mdelay(100);

        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_check_progmod
Input   :
Output  :
function    :   check if MCU at progmode or not
***********************************************************************************************/
int icn83xx_check_progmod(void)
{
        int ret;
        unsigned char ucTemp = 0x0;
        ret = icn83xx_prog_i2c_rxdata(0x0, &ucTemp, 1);
        flash_info("icn83xx_check_progmod: 0x%x\n", ucTemp);
        if(ret < 0) {
                flash_error("icn83xx_check_progmod error, ret: %d\n", ret);
                return ret;
        }

        return 0;
}


/***********************************************************************************************
Name    :   icn83xx_uu
Input   :
Output  :
function    :   unlock flash
***********************************************************************************************/
int icn83xx_uu(void)
{
        unsigned char ucTemp = 0x0;
        ucTemp = 0x1e;
        icn83xx_prog_i2c_txdata(0x050a, &ucTemp, 1);
        ucTemp = 0x10;
        icn83xx_prog_i2c_txdata(0x050b, &ucTemp, 1);

        return 0;
}
/***********************************************************************************************
Name    :   icn83xx_ll
Input   :
Output  :
function    :   lock flash
***********************************************************************************************/
int icn83xx_ll(void)
{
        unsigned char ucTemp = 0x0;
        ucTemp = 0xcc;
        icn83xx_prog_i2c_txdata(0x050a, &ucTemp, 1);
        ucTemp = 0xcc;
        icn83xx_prog_i2c_txdata(0x050b, &ucTemp, 1);

        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_op1
Input   :
Output  :
function    :   erase flash
***********************************************************************************************/

int  icn83xx_op1(char info, unsigned short offset, unsigned int size)
{
        int count = 0;
        unsigned char ucTemp = 0x0;
        unsigned short uiAddress = 0x0;
        int i;

        icn83xx_uu();
        for(i=0; i<size; ) {
                uiAddress = offset + i;
                // flash_info("uiAddress: 0x%x\n", uiAddress);
                ucTemp = U16LOBYTE(uiAddress);
                icn83xx_prog_i2c_txdata(0x0502, &ucTemp, 1);
                ucTemp = U16HIBYTE(uiAddress);
                icn83xx_prog_i2c_txdata(0x0503, &ucTemp, 1);

                ucTemp = 0x02;
                icn83xx_prog_i2c_txdata(0x0500, &ucTemp, 1);
                ucTemp = 0x01;
                count = 0;
                while(ucTemp) {
                        icn83xx_prog_i2c_rxdata(0x0501, &ucTemp, 1);
                        count++;
                        if(count > 5000) {
                                flash_error("op1 ucTemp: 0x%x\n", ucTemp);
                                return 1;
                        }
                }
                i = i+1024;
        }
        icn83xx_ll();
        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_op2
Input   :
Output  :
function    :   progm flash
***********************************************************************************************/
int  icn83xx_op2(char info, unsigned short offset, unsigned char * buffer, unsigned int size)
{
        int count = 0;
        unsigned int flash_size;
        unsigned char ucTemp;
        unsigned short uiAddress;
        ucTemp = 0x00;
        uiAddress = 0x1000;

        icn83xx_prog_i2c_txdata(uiAddress, buffer, size);

        icn83xx_uu();

        ucTemp = U16LOBYTE(offset);
        icn83xx_prog_i2c_txdata(0x0502, &ucTemp, 1);
        ucTemp = U16HIBYTE(offset);
        icn83xx_prog_i2c_txdata(0x0503, &ucTemp, 1);

        icn83xx_prog_i2c_txdata(0x0504,(char *)&uiAddress, 2);


        //ensure size is even
        if(size%2 != 0) {
                flash_info("write op size: %d\n", size);
                flash_size = size+1;
        }
        else
                flash_size = size;

        ucTemp = U16LOBYTE(flash_size);
        icn83xx_prog_i2c_txdata(0x0506, &ucTemp, 1);
        ucTemp = U16HIBYTE(flash_size);
        icn83xx_prog_i2c_txdata(0x0507, &ucTemp, 1);
        ucTemp = 0x01;

        if(info > 0)
               ucTemp = 0x01 | (1<<3);

        icn83xx_prog_i2c_txdata(0x0500, &ucTemp, 1);
        while(ucTemp) {
                icn83xx_prog_i2c_rxdata(0x0501, &ucTemp, 1);
                count++;
                if(count > 5000) {
                        flash_error("op2 ucTemp: 0x%x\n", ucTemp);
                        return 1;
                }

        }
        icn83xx_ll();

        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_op3
Input   :
Output  :
function    :   read flash
***********************************************************************************************/
int  icn83xx_op3(char info, unsigned short offset, unsigned char * buffer, unsigned int size)
{
        int count = 0;
        unsigned int flash_size;
        unsigned char ucTemp;
        unsigned short uiAddress;
        ucTemp = 0x00;
        uiAddress = 0x1000;
        icn83xx_uu();
        ucTemp = U16LOBYTE(offset);
        icn83xx_prog_i2c_txdata(0x0502, &ucTemp, 1);
        ucTemp = U16HIBYTE(offset);
        icn83xx_prog_i2c_txdata(0x0503, &ucTemp, 1);

        icn83xx_prog_i2c_txdata(0x0504, (unsigned char*)&uiAddress, 2);

        //ensure size is even
        if(size%2 != 0) {
                flash_info("read op size: %d\n", size);
                flash_size = size+1;
        }
        else
                flash_size = size;

        ucTemp = U16LOBYTE(flash_size);
        icn83xx_prog_i2c_txdata(0x0506, &ucTemp, 1);

        ucTemp = U16HIBYTE(flash_size);
        icn83xx_prog_i2c_txdata(0x0507, &ucTemp, 1);
        ucTemp = 0x40;

        if(info > 0)
                ucTemp = 0x40 | (1<<3);

        icn83xx_prog_i2c_txdata(0x0500, &ucTemp, 1);
        ucTemp = 0x01;
        while(ucTemp) {
                icn83xx_prog_i2c_rxdata(0x0501, &ucTemp, 1);
                count++;
                if(count > 5000) {
                        flash_error("op3 ucTemp: 0x%x\n", ucTemp);
                        return 1;
                }

        }
        icn83xx_ll();
        icn83xx_prog_i2c_rxdata(uiAddress, buffer, size);

        return 0;
}


/***********************************************************************************************
Name    :   icn83xx_goto_nomalmode
Input   :
Output  :
function    :   when prog flash ok, change flash info flag
***********************************************************************************************/
int icn83xx_goto_nomalmode(void)
{
        int ret = -1;
        //unsigned short addr = 0;
        char temp_buf[3];

        flash_info("icn83xx_goto_nomalmode\n");
        temp_buf[0] = 0x03;
        icn83xx_prog_i2c_txdata(0x0f00, temp_buf, 1);

        msleep(100);
/*
        addr = 0;
        temp_buf[0] = U16HIBYTE(addr);
        temp_buf[1] = U16LOBYTE(addr);
        temp_buf[2] = 0;
        ret = icn83xx_i2c_txdata(230, temp_buf, 2);
        if (ret < 0) {
            pr_err("write reg failed! ret: %d\n", ret);
            return -1;
        }

        icn83xx_i2c_rxdata(232, &temp_buf[2], 1);
        flash_info("temp_buf[2]: 0x%x\n", temp_buf[2]);
*/
        ret = icn83xx_readInfo(0, &temp_buf[2]);
        if(ret != 0)
                return ret;
        flash_info("temp_buf[2]: 0x%x\n", temp_buf[2]);
        if(temp_buf[2] == 0xff) {
/*
                addr = 0;
                temp_buf[0] = U16HIBYTE(addr);
                temp_buf[1] = U16LOBYTE(addr);
                ret = icn83xx_i2c_txdata(230, temp_buf, 2);
                if (ret < 0) {
                    pr_err("write reg failed! ret: %d\n", ret);
                    return -1;
                }
                temp_buf[0] = 0x11;
                ret = icn83xx_i2c_txdata(232, temp_buf, 1);
                if (ret < 0) {
                    pr_err("write reg failed! ret: %d\n", ret);
                    return -1;
                }
*/
                ret = icn83xx_writeInfo(0, 0x11);
                if(ret != 0)
                        return ret;

        }

        return 0;
}

/***********************************************************************************************
Name    :   icn83xx_read_fw_Ver
Input   :   fw
Output  :
function    :   read fw version
***********************************************************************************************/

short  icn83xx_read_fw_Ver(char *fw)
{
        short FWversion;
        char tmp[2];
        int file_size;
        file_size = icn83xx_open_fw(fw);
        if(file_size < 0) {
                return -1;
        }
        icn83xx_read_fw(0x4000, 2, &tmp[0]);

        icn83xx_close_fw();
        FWversion = (tmp[0]<<8)|tmp[1];
        //flash_info("FWversion: 0x%x\n", FWversion);
        return FWversion;
}




/***********************************************************************************************
Name    :   icn83xx_fw_update
Input   :   fw
Output  :
function    :   upgrade fw
***********************************************************************************************/

E_UPGRADE_ERR_TYPE  icn83xx_fw_update(char *fw)
{
        int file_size, last_length;
        int j, num;
        int checksum_bak = 0;
        int checksum = 0;
        char temp_buf[B_SIZE];
#ifdef ENABLE_BYTE_CHECK
        char temp_buf1[B_SIZE];
#endif

        file_size = icn83xx_open_fw(fw);
        if(file_size < 0) {
                icn83xx_update_status(R_FILE_ERR);
                return R_FILE_ERR;
        }

        if(icn83xx_goto_progmode() != 0) {
                if(icn83xx_check_progmod() < 0) {
                        icn83xx_update_status(R_STATE_ERR);
                        icn83xx_close_fw();
                        return R_STATE_ERR;
                }
        }
        //msleep(50);

        if(icn83xx_op1(0, 0, file_size) != 0) {
                flash_error("icn83xx_op1 error\n");
                icn83xx_update_status(R_ERASE_ERR);
                icn83xx_close_fw();
                return R_ERASE_ERR;
        }
        icn83xx_update_status(5);

        num = file_size/B_SIZE;
        for(j=0; j < num; j++) {
                icn83xx_read_fw(j*B_SIZE, B_SIZE, temp_buf);

                // icn83xx_op3(0, j*B_SIZE, temp_buf1, B_SIZE);
                //  icn83xx_memdump(temp_buf1, B_SIZE);

                if(icn83xx_op2(0, j*B_SIZE, temp_buf, B_SIZE) != 0) {
                        icn83xx_update_status(R_PROGRAM_ERR);
                        icn83xx_close_fw();
                        return R_PROGRAM_ERR;
                }
                checksum_bak = icn83xx_checksum(checksum_bak, temp_buf, B_SIZE);

                icn83xx_update_status(5+(int)(60*j/num));
        }
        last_length = file_size - B_SIZE*j;
        if(last_length > 0) {
                icn83xx_read_fw(j*B_SIZE, last_length, temp_buf);

                // icn83xx_op3(0, j*B_SIZE, temp_buf1, B_SIZE);
                // icn83xx_memdump(temp_buf1, B_SIZE);

                if(icn83xx_op2(0, j*B_SIZE, temp_buf, last_length) != 0) {
                        icn83xx_update_status(R_PROGRAM_ERR);
                        icn83xx_close_fw();
                        return R_PROGRAM_ERR;
                }
                checksum_bak = icn83xx_checksum(checksum_bak, temp_buf, last_length);
        }

        icn83xx_close_fw();
        icn83xx_update_status(65);

#ifdef ENABLE_BYTE_CHECK
        file_size = icn83xx_open_fw(fw);
        num = file_size/B_SIZE;
#endif

        for(j=0; j < num; j++) {

#ifdef ENABLE_BYTE_CHECK
                icn83xx_read_fw(j*B_SIZE, B_SIZE, temp_buf1);
#endif
                icn83xx_op3(0, j*B_SIZE, temp_buf, B_SIZE);
                checksum = icn83xx_checksum(checksum, temp_buf, B_SIZE);

#ifdef ENABLE_BYTE_CHECK
                if(memcmp(temp_buf1, temp_buf, B_SIZE) != 0) {
                        flash_error("cmp error, %d\n", j);
                        icn83xx_memdump(temp_buf1, B_SIZE);
                        icn83xx_memdump(temp_buf, B_SIZE);
                        icn83xx_update_status(R_VERIFY_ERR);
#ifdef ENABLE_BYTE_CHECK
                        icn83xx_close_fw();
#endif
                        return R_VERIFY_ERR;
                       //while(1);
                }
#endif
                icn83xx_update_status(65+(int)(30*j/num));
        }

#ifdef ENABLE_BYTE_CHECK
        last_length = file_size - B_SIZE*j;
#endif
        if(last_length > 0) {
#ifdef ENABLE_BYTE_CHECK
                icn83xx_read_fw(j*B_SIZE, last_length, temp_buf1);
#endif
                icn83xx_op3(0, j*B_SIZE, temp_buf, last_length);
                checksum = icn83xx_checksum(checksum, temp_buf, last_length);

#ifdef ENABLE_BYTE_CHECK
                if(memcmp(temp_buf1, temp_buf, last_length) != 0) {
                        flash_error("cmp error, %d\n", j);
                        icn83xx_memdump(temp_buf1, last_length);
                        icn83xx_memdump(temp_buf, last_length);
                        icn83xx_update_status(R_VERIFY_ERR);
#ifdef ENABLE_BYTE_CHECK
                        icn83xx_close_fw();
#endif
                        return R_VERIFY_ERR;
                        //while(1);
                }
#endif

        }

#ifdef ENABLE_BYTE_CHECK
        icn83xx_close_fw();
#endif

        flash_info("checksum_bak: 0x%x, checksum: 0x%x\n", checksum_bak, checksum);
        if(checksum_bak != checksum) {
                flash_error("upgrade checksum error\n");
                icn83xx_update_status(R_VERIFY_ERR);
                return R_VERIFY_ERR;
        }

        if(icn83xx_goto_nomalmode() != 0) {
                flash_error("icn83xx_goto_nomalmode error\n");
                icn83xx_update_status(R_STATE_ERR);
                return R_STATE_ERR;
        }

        icn83xx_update_status(R_OK);
        flash_info("upgrade ok\n");
        return R_OK;
}
