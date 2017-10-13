/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "efex_queue.h"
#include "buf_queue.h"
#include <malloc.h>

extern int sunxi_sprite_write(uint start_block, uint nblock, void *buffer);
buf_element_t      buf_queue_element;

int efex_queue_init(void)
{
    int page_size = 0;
    if(buf_queue_init())
    {
        return -1;
    }
    
   
    //buf_queue_get_page_size() function should be call   after buf_queue_init function
    page_size = buf_queue_get_page_size();
    if( page_size == 0)
    {
        printf("efex queue init fail:make sure buf_queue_init function has be called\n");
        return -1;
    }

    //alloc mem for gloab element,    this memory for efex page write
    memset(&buf_queue_element, 0 , sizeof(buf_element_t));
    buf_queue_element.buff = malloc(page_size);
    if(buf_queue_element.buff == NULL) 
    {
        printf("efex_queue_init error: malloc memory size=0x%x fail\n",page_size);
        return -1;
    }

    return 0;

}

int efex_queue_exit(void)
{
    
    //free global element buff
    if(buf_queue_element.buff) 
    {
        free(buf_queue_element.buff);
    }
    return buf_queue_exit();
}

int efex_queue_write_one_page( void )
{
    if(buf_queue_empty())
    {
        //printf("efex enqueue empty\n");
        return 0;
    }
    buf_dequeue(&buf_queue_element);
    
    if(!sunxi_sprite_write(buf_queue_element.addr, buf_queue_element.sector_num,
        (void *)buf_queue_element.buff))
    {
       printf("efex_queue_write_one_page error: write flash from 0x%x, sectors 0x%x failed\n", 
        buf_queue_element.addr,buf_queue_element.sector_num);
       return -1;
    }
    return 0;
}

int efex_queue_write_all_page( void )
{
    if(buf_queue_empty())
    {
        //printf("efex queue empty\n");
        return 0;
    }
    while(buf_dequeue(&buf_queue_element) == 0)
    {
        if(!sunxi_sprite_write(buf_queue_element.addr, buf_queue_element.sector_num,
            (void *)buf_queue_element.buff))
        {
            printf("efex_queue_write_one_page error: write flash from 0x%x, sectors 0x%x failed\n", 
                buf_queue_element.addr,buf_queue_element.sector_num);
            return -1;
        }
    }
    //printf("write all page done\n");
    return 0;
}


int efex_save_buff_to_queue(uint flash_start, uint flash_sectors, void* buff)
{
    int sec_per_page;     
    int queue_free_page;
    int offset;
    buf_element_t element;
    int require_page ;
    

    //make sure queue has enough space to save buffer
    sec_per_page     = buf_queue_get_page_size()>>9;
    require_page = (flash_sectors+sec_per_page-1)/sec_per_page;
    
    queue_free_page   = buf_queue_free_size();
    if(require_page > queue_free_page) 
    {
        int i = 0;
        for(i = 0; i < require_page - queue_free_page; i++)
        {
            if(efex_queue_write_one_page())
            {
                return -1;
            }
        }
    }

    if(buf_queue_free_size() < require_page)
    {
        printf("efex queue error: free space not enough\n");
        return -1;
    }

    //save buff to queue
    offset = 0;
    while(flash_sectors > sec_per_page)
    {
        element.addr = flash_start;
        element.sector_num = sec_per_page;
        element.buff = (u8*)(((u8*)buff)+offset*512);
        buf_enqueue(&element);
        flash_sectors -= sec_per_page;
        offset += sec_per_page;
        flash_start += sec_per_page;
    }
    if(flash_sectors)
    {
        element.addr = flash_start;
        element.sector_num = flash_sectors;
        element.buff = (u8*)(((u8*)buff)+offset*512);
        buf_enqueue(&element);
    }

    return 0;
}

