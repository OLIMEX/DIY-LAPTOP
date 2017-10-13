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

#include "buf_queue.h"
#include <malloc.h>

//extern __u32 NAND_GetPageSize(void);
extern __u32 NAND_GetLogicPageSize(void);


int                buf_queue_init_flag = 0;
int                buf_queue_max_len  = 80;
int                buf_queue_page_size = 8*1024;
int                buf_queue_current_len = 0;
u8*                buf_queue_base_buf = NULL;

buf_node_t *       buf_queue_head = NULL;
buf_node_t *       buf_queue_tail = NULL;



int buf_queue_init(void)
{
    int i = 0;

    if(buf_queue_init_flag)
    {
        printf("sunxi efex queue error: already init\n");
        return -1;
    }
    
    //init queue page size and max len by storage type
    int storage_type = uboot_spare_head.boot_data.storage_type;
    if(storage_type == 0)
    {
        buf_queue_page_size = NAND_GetLogicPageSize();
        buf_queue_max_len = 80;
    }
    else
    {
        buf_queue_page_size = 64*1024;
        buf_queue_max_len = 20;
    }
    printf("buf queue page size = %d\n", buf_queue_page_size);

    buf_queue_base_buf = NULL;
    buf_queue_head = buf_queue_tail = NULL;

    //malloc queue base buff
    buf_queue_base_buf = ( u8*) malloc(buf_queue_page_size*buf_queue_max_len);
    if(buf_queue_base_buf == NULL) 
    {
        printf("sunxi usb efex queue error: malloc memory fail size 0x%x\n",
            buf_queue_page_size*buf_queue_max_len);
        return -1;
    }

    //create cycle queue
    buf_element_t element;
    memset(&element, 0 , sizeof(element));
    for(i = 0; i < buf_queue_max_len ; i++)
    {
        //malloc page memory from queue base_buf
        element.buff = buf_queue_base_buf+i*buf_queue_page_size ;
  
        //malloc node memory
        buf_node_t *node = ( buf_node_t*) malloc(sizeof(buf_node_t));
        if(node == NULL) 
        {
             printf("sunxi usb efex queue error: malloc memory fail size 0x%zu\n",sizeof(buf_node_t));
            return -1;
        }
        node->element= element;
        node->next = NULL;
        
        if(buf_queue_tail == NULL)
        {
            //queue is empty
            buf_queue_head = buf_queue_tail = node ;
        }
        else
        {
            buf_queue_tail->next = node;
            buf_queue_tail = buf_queue_tail->next;
        }
    }

    //set cycle queue 
    buf_queue_tail->next = buf_queue_head;
    
    //set head and tail point to the same node when begin
    buf_queue_head = buf_queue_tail;
    
    //set current len to zero
    buf_queue_current_len = 0;

    //set init flag
    buf_queue_init_flag = 1;
    return 0;
}

int buf_queue_exit(void)
{
    buf_node_t *tmp = NULL;
    int free_cnt = buf_queue_max_len;
    //free base buf
    if(buf_queue_base_buf)
    {
        free(buf_queue_base_buf);
        buf_queue_base_buf = NULL;
    }
    //free node buf
    while(buf_queue_head != NULL)
    {
        tmp = buf_queue_head;
        
        //move to next node
        buf_queue_head = buf_queue_head->next;

        //free queue node memory
        free(tmp);
        
        //check 
        free_cnt--;
        if(free_cnt == 0) break;
    }

    buf_queue_head = buf_queue_tail = NULL;
    buf_queue_current_len = 0;
    buf_queue_init_flag = 0;
    return 0;
    
}

int buf_queue_empty(void)
{
    return buf_queue_current_len == 0 ?1:0;
}

int buf_queue_full(void)
{
    return buf_queue_current_len == buf_queue_max_len?1:0;
}

int buf_queue_free_size(void)
{
    return buf_queue_max_len-buf_queue_current_len;
}

int buf_queue_get_page_size(void)
{
    return buf_queue_page_size;
}

int buf_enqueue(buf_element_t* element)
{
    if(buf_queue_full())
    {
        //printf("efex queue: full");
        //full queue
        return -1;
    }
    buf_element_t *pelem = NULL;
    if(buf_queue_empty())
    {
        pelem = &(buf_queue_head->element);
    }
    else
    {
        pelem = &(buf_queue_tail->element);
    }
    
    memcpy(pelem->buff,element->buff, element->sector_num*512);
    pelem->addr = element->addr ;
    pelem->sector_num = element->sector_num;

    buf_queue_tail = buf_queue_tail->next;

    buf_queue_current_len++;

    return 0;
}

int buf_dequeue(buf_element_t* pelement)
{
    if(buf_queue_empty()) return -1; //empty queue
    //get value
    buf_element_t *ptmp = &buf_queue_head->element;
    pelement->addr = ptmp->addr;
    pelement->sector_num = ptmp->sector_num;
    memcpy(pelement->buff,ptmp->buff,ptmp->sector_num*512);
    
    //move head
    buf_queue_head = buf_queue_head->next;
    
    buf_queue_current_len--;
    
    //printf("efex dequeue ok: addr0x%x, sector 0x%x \n",pelement->addr,pelement->sector_num);
    return 0;
}
